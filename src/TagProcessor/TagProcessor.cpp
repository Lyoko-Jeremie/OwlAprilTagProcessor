// jeremie

#include "TagProcessor.h"

#include "../GetImage/GetImage.h"
#include "../SendResult/SendResult.h"
#include "../AprilTagData/AprilTagData.h"

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
//        ptr_GetImage_->get();

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

} // OwlTagProcessor