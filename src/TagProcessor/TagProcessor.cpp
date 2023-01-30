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
                    time_loop(ec);
                }
        );
    }

    void TagProcessor::time_loop(const boost::system::error_code &ec) {
        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "TagProcessor time_loop ec: " << ec.what();
            // ignore
            return;
        }

        // TODO do task
        ptr_GetImage_->timeoutMs = ptr_TagConfigLoader_->config.configGetImage.timeoutMs;
        ptr_GetImage_->get(
                ptr_TagConfigLoader_->config.configGetImage.host,
                ptr_TagConfigLoader_->config.configGetImage.port,
                ptr_TagConfigLoader_->config.configGetImage.target,
                ptr_TagConfigLoader_->config.configGetImage.version,
                [this, self = shared_from_this()]
                        (boost::beast::error_code ec, bool ok, cv::Mat img) {
                    if (ec) {
                        // TODO check stop or skip
                        return;
                    }
                    if (!ok) {
                        // TODO skip
                        return;
                    }

                    auto r = ptr_AprilTagData_->analysis(std::move(img));
                    auto o = AprilTagDataObject2Map(r);

                    ptr_SendResult_->timeoutMs = ptr_TagConfigLoader_->config.configSendResult.timeoutMs;
                    ptr_SendResult_->send(
                            ptr_TagConfigLoader_->config.configSendResult.host,
                            ptr_TagConfigLoader_->config.configSendResult.port,
                            ptr_TagConfigLoader_->config.configSendResult.target,
                            ptr_TagConfigLoader_->config.configSendResult.version,
                            o,
                            [this, self = shared_from_this()]
                                    (boost::beast::error_code ec, bool ok) {
                                if (ec) {
                                    // TODO check stop or skip
                                    return;
                                }
                                if (!ok) {
                                    // TODO skip
                                    return;
                                }
                                // https://www.boost.org/doc/libs/1_81_0/doc/html/boost_asio/tutorial/tuttimer3.html
                                // timer_.expires_at(timer_.expiry() + boost::asio::chrono::milliseconds(msTimer_));
                                timer_->expires_from_now(boost::asio::chrono::milliseconds(timeDurationMs_));

                                timer_->async_wait(
                                        [this, self = shared_from_this()]
                                                (const boost::system::error_code &ec) {
                                            time_loop(ec);
                                        }
                                );
                            }
                    );
                }
        );

    }

} // OwlTagProcessor
