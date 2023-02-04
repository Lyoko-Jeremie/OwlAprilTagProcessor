// jeremie

#include "TagProcessor.h"

#include <utility>
#include <atomic>
#include <boost/circular_buffer.hpp>

namespace OwlTagProcessor {

    class FpsTracer : std::enable_shared_from_this<FpsTracer> {
        std::atomic_int count{0};
        boost::asio::io_context &ioc_;
        boost::asio::steady_timer timer_;

        double averageFps = 0;
        const double averageWindow = 10;

        boost::circular_buffer<double> averageFpsBuffer{static_cast<size_t>(averageWindow), 0};

        boost::circular_buffer<double> averageWastBuffer{static_cast<size_t>(averageWindow), 0};
        std::mutex mtxAverageWastBuffer;

    public:
        explicit FpsTracer(
                boost::asio::io_context &ioc
        ) : ioc_(ioc),
            timer_(ioc_, boost::asio::chrono::seconds(1)) {
            timer_.async_wait([this](const boost::system::error_code &ec) { flushFps(ec); });
            BOOST_LOG_TRIVIAL(trace) << "FpsTracer averageWindow:" << averageWindow;
        }

        void frame(boost::asio::chrono::milliseconds wastMs) {
            ++count;
            std::lock_guard lg{mtxAverageWastBuffer};
            averageWastBuffer.push_back(static_cast<double>(wastMs.count()));
        }

        void flushFps(const boost::system::error_code &ec) {
            if (ec) {
                BOOST_LOG_TRIVIAL(error) << "FpsTracer flushFps ec: " << ec << ec.what();
                return;
            }
            auto f = count.exchange(0);
            averageFpsBuffer.push_back(f);
            double a10 = std::reduce(averageFpsBuffer.begin(), averageFpsBuffer.end()) / averageWindow;

            double w0 = std::reduce(averageWastBuffer.begin(), averageWastBuffer.end()) / averageWindow;

            // avg = avg*((1-w)/w) + new*(1/w) = ((avg*(1-w) + new*(1)) / w
            averageFps = (averageFps * (averageWindow - 1) + double(f)) / averageWindow;
            BOOST_LOG_TRIVIAL(trace) << "Fps/1: " << f
                                     << "\tFps/arg/" << averageWindow << ": " << averageFps
                                     << "\tms:" << (1000.0 / averageFps)
                                     << "\tFps/" << averageWindow << ": " << a10
                                     << "\tms:" << (1000.0 / a10)
                                     << "\tms/w/" << averageWindow << ": " << w0;

            timer_.expires_at(timer_.expiry() + boost::asio::chrono::seconds(1));
            timer_.async_wait([this](const boost::system::error_code &ec) { flushFps(ec); });
        }

    };

    void TagProcessor::start() {
        if (timer_) {
            timer_->cancel();
            timer_ = nullptr;
        }
        timer_ = std::make_shared<boost::asio::steady_timer>(ioc_, boost::asio::chrono::milliseconds(timeStartMs_));
        timer_->async_wait(
                [this, self = shared_from_this()]
                        (const boost::system::error_code &ec) {
                    to_get_image(ec);
                }
        );
    }

    void TagProcessor::to_get_image(const boost::system::error_code &ec) {
        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "TagProcessor to_get_image ec: " << ec << ec.what();
            // ignore
            return;
        }

        // do task
        ptr_GetImage_->timeoutMs = ptr_TagConfigLoader_->config.configGetImage.timeoutMs;
        ptr_GetImage_->get(
                ptr_TagConfigLoader_->config.configGetImage.host,
                ptr_TagConfigLoader_->config.configGetImage.port,
                ptr_TagConfigLoader_->config.configGetImage.target,
                ptr_TagConfigLoader_->config.configGetImage.version,
                [this, self = shared_from_this()]
                        (boost::beast::error_code ec, bool ok, cv::Mat img) {
                    boost::asio::post(ioc_, [this, self = shared_from_this(), ec, ok, img]() {

                        if (ec) {
                            // check stop or skip
                            if (ec == boost::beast::error::timeout) {
                                ++timeoutCount_;
                                if (timeoutCount_ > timeoutCountLimit_) {
                                    // too many continue timeout
                                    timeoutCount_ = timeoutCountLimit_;
                                    return restart_to_next_loop(ec);
                                }
                                return skip_to_next_loop();
                            }
                            return restart_to_next_loop(ec);
                        }
                        if (!ok) {
                            // skip
                            return skip_to_next_loop();
                        }
                        timeoutCount_ = 0;

                        if (img.empty()) {
                            BOOST_LOG_TRIVIAL(trace) << "to_get_image (img.empty()). ";
                            // skip
                            return skip_to_next_loop();
                        }

                        to_analysis_april_tag(img);

                    });

                }
        );

    }

    void TagProcessor::to_analysis_april_tag(cv::Mat img) {
        auto r = ptr_AprilTagData_->analysis(std::move(img));
        auto o = AprilTagDataObject2ResultType(r);

        to_send_result(std::move(o));
    }

    void TagProcessor::to_send_result(std::shared_ptr<OwlAprilTagData::AprilTagResultType> o) {

        ptr_SendResult_->timeoutMs = ptr_TagConfigLoader_->config.configSendResult.timeoutMs;
        ptr_SendResult_->send(
                ptr_TagConfigLoader_->config.configSendResult.host,
                ptr_TagConfigLoader_->config.configSendResult.port,
                ptr_TagConfigLoader_->config.configSendResult.target,
                ptr_TagConfigLoader_->config.configSendResult.version,
                o,
                [this, self = shared_from_this(), o]
                        (boost::beast::error_code ec, bool ok) {
                    boost::asio::post(ioc_, [this, self = shared_from_this(), ec, ok]() {

                        if (ec) {
                            // check stop or skip
                            if (ec == boost::beast::error::timeout) {
                                ++timeoutCount_;
                                if (timeoutCount_ > timeoutCountLimit_) {
                                    // too many continue timeout
                                    timeoutCount_ = timeoutCountLimit_;
                                    return restart_to_next_loop(ec);
                                }
                                return skip_to_next_loop();
                            }
                            return restart_to_next_loop(ec);
                        }
                        if (!ok) {
                            // skip
                            return skip_to_next_loop();
                        }
                        timeoutCount_ = 0;

                        next_loop();

                    });
                }
        );

    }

    void TagProcessor::skip_to_next_loop() {

        // https://www.boost.org/doc/libs/1_81_0/doc/html/boost_asio/tutorial/tuttimer3.html
        // timer_.expires_at(timer_.expiry() + boost::asio::chrono::milliseconds(msTimer_));
        // timer_->expires_from_now(boost::asio::chrono::milliseconds(timeDurationMs_));
        setNextDuration(boost::asio::chrono::milliseconds(timeDurationMs_));

        timer_->async_wait(
                [this, self = shared_from_this()]
                        (const boost::system::error_code &ec) {
                    to_get_image(ec);
                }
        );
    }

    void TagProcessor::restart_to_next_loop(const boost::system::error_code &ec) {
//        if (ec
//            && ec != boost::beast::error::timeout
//            && ec != boost::beast::http::error::end_of_stream
//            && ec != boost::beast::http::error::end_of_chunk
//            && ec != boost::beast::http::error::bad_content_length
//            && ec != boost::beast::http::error::bad_line_ending
//            && ec != boost::beast::http::error::bad_method
//            && ec != boost::beast::http::error::bad_reason
//            && ec != boost::beast::http::error::unexpected_body
//            && ec != boost::asio::error::connection_refused
//            && ec != boost::asio::error::connection_aborted
//            && ec != boost::asio::error::connection_reset
//                ) {
//            BOOST_LOG_TRIVIAL(error) << "restart_to_next_loop stop ec: " << ec << " " << ec.what();
//            timer_->cancel();
//            return;
//        }
        if (ec) {
            // ignore error, only log it
            BOOST_LOG_TRIVIAL(error) << "restart_to_next_loop stop ec: " << ec << " " << ec.what();
//            timer_->cancel();
//            return;
        }
        BOOST_LOG_TRIVIAL(trace) << "restart_to_next_loop skip. ";

        // https://www.boost.org/doc/libs/1_81_0/doc/html/boost_asio/tutorial/tuttimer3.html
        // timer_.expires_at(timer_.expiry() + boost::asio::chrono::milliseconds(msTimer_));
        // timer_->expires_from_now(boost::asio::chrono::milliseconds(timeStartMs_));
        setNextDuration(boost::asio::chrono::milliseconds(timeDurationMs_));

        timer_->async_wait(
                [this, self = shared_from_this()]
                        (const boost::system::error_code &ec) {
                    to_get_image(ec);
                }
        );
    }

    void TagProcessor::next_loop() {
        // https://www.boost.org/doc/libs/1_81_0/doc/html/boost_asio/tutorial/tuttimer3.html
        // timer_.expires_at(timer_.expiry() + boost::asio::chrono::milliseconds(msTimer_));
        // timer_->expires_from_now(boost::asio::chrono::milliseconds(timeDurationMs_));
        setNextDuration(boost::asio::chrono::milliseconds(timeDurationMs_));

        timer_->async_wait(
                [this, self = shared_from_this()]
                        (const boost::system::error_code &ec) {
                    to_get_image(ec);
                }
        );
    }

    TagProcessor::TagProcessor(boost::asio::io_context &ioc, std::shared_ptr<OwlGetImage::GetImage> ptr_GetImage,
                               std::shared_ptr<OwlSendResult::SendResult> ptr_SendResult,
                               std::shared_ptr<OwlAprilTagData::AprilTagData> ptr_AprilTagData,
                               std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> TagConfigLoader)
            : ioc_(ioc),
              ptr_GetImage_(std::move(ptr_GetImage)),
              ptr_SendResult_(std::move(ptr_SendResult)),
              ptr_AprilTagData_(std::move(ptr_AprilTagData)),
              ptr_TagConfigLoader_(std::move(TagConfigLoader)),
              timeStartMs_(ptr_TagConfigLoader_->config.configTagProcessor.timeStartMs),
              timeDurationMs_(ptr_TagConfigLoader_->config.configTagProcessor.timeDurationMs),
              timeoutCountLimit_(ptr_TagConfigLoader_->config.configTagProcessor.timeoutCountLimit),
              fpsTracer(std::make_shared<FpsTracer>(ioc_)) {
    }

    void TagProcessor::setNextDuration(boost::asio::chrono::milliseconds requestDuration) {
        // https://www.boost.org/doc/libs/1_81_0/doc/html/boost_asio/tutorial/tuttimer3.html
        // timer_.expires_at(timer_.expiry() + boost::asio::chrono::milliseconds(msTimer_));
        // timer_->expires_from_now(boost::asio::chrono::milliseconds(timeDurationMs_));

        fpsTracer->frame(
                boost::asio::chrono::duration_cast<boost::asio::chrono::milliseconds>(
                        boost::asio::chrono::steady_clock::now() - timer_->expiry()
                )
        );
        if (timer_->expiry() + requestDuration > boost::asio::chrono::steady_clock::now()) {
            // timer_->expires_after(timer_->expiry() + requestDuration - boost::asio::chrono::steady_clock::now());
            timer_->expires_from_now(timer_->expiry() + requestDuration - boost::asio::chrono::steady_clock::now());
        } else {
            // timer_->expires_after(
            //         boost::asio::chrono::milliseconds(
            //                 ptr_TagConfigLoader_->config.configTagProcessor.timeDurationMini
            //         )
            // );
            timer_->expires_from_now(
                    boost::asio::chrono::milliseconds(
                            ptr_TagConfigLoader_->config.configTagProcessor.timeDurationMini
                    )
            );
        }
    }

} // OwlTagProcessor
