// jeremie

#include "TagProcessor.h"

#include <utility>


namespace OwlTagProcessor {

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
            BOOST_LOG_TRIVIAL(error) << "TagProcessor to_get_image ec: " << ec;
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
        auto r = ptr_AprilTagData_->analysis(img);
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
        timer_->expires_from_now(boost::asio::chrono::milliseconds(timeDurationMs_));

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
        timer_->expires_from_now(boost::asio::chrono::milliseconds(timeStartMs_));

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
        timer_->expires_from_now(boost::asio::chrono::milliseconds(timeDurationMs_));

        timer_->async_wait(
                [this, self = shared_from_this()]
                        (const boost::system::error_code &ec) {
                    to_get_image(ec);
                }
        );
    }

} // OwlTagProcessor
