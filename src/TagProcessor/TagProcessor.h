// jeremie

#ifndef OWLAPRILTAGPROCESSOR_TAGPROCESSOR_H
#define OWLAPRILTAGPROCESSOR_TAGPROCESSOR_H

#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include "../ConfigLoader/TagConfigLoader.h"

#include "../GetImage/GetImage.h"
#include "../SendResult/SendResult.h"
#include "../AprilTagData/AprilTagData.h"

namespace OwlTagProcessor {

    class FpsTracer;

    class TagProcessor : public std::enable_shared_from_this<TagProcessor> {
    public:
        TagProcessor(
                boost::asio::io_context &ioc,
                std::shared_ptr<OwlGetImage::GetImage> ptr_GetImage,
                std::shared_ptr<OwlSendResult::SendResult> ptr_SendResult,
                std::shared_ptr<OwlAprilTagData::AprilTagData> ptr_AprilTagData,
                std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> TagConfigLoader
        );

    private:
        boost::asio::io_context &ioc_;

        std::shared_ptr<OwlGetImage::GetImage> ptr_GetImage_;
        std::shared_ptr<OwlSendResult::SendResult> ptr_SendResult_;
        std::shared_ptr<OwlAprilTagData::AprilTagData> ptr_AprilTagData_;

        std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> ptr_TagConfigLoader_;

        long timeStartMs_ = 1000;
        long timeDurationMs_ = 300;

        std::shared_ptr<boost::asio::steady_timer> timer_;

        size_t timeoutCount_ = 0;
        const size_t timeoutCountLimit_ = 6;

        std::shared_ptr<FpsTracer> fpsTracer;
    public:
        void start();

    private:

        void to_get_image(const boost::system::error_code &ec);

        void to_analysis_april_tag(cv::Mat img);

        void to_send_result(std::shared_ptr<OwlAprilTagData::AprilTagResultType> o);

        void skip_to_next_loop();

        void restart_to_next_loop(const boost::system::error_code &ec);

        void next_loop();

        void setNextDuration(
                boost::asio::chrono::milliseconds requestDuration
        ) {
            // https://www.boost.org/doc/libs/1_81_0/doc/html/boost_asio/tutorial/tuttimer3.html
            // timer_.expires_at(timer_.expiry() + boost::asio::chrono::milliseconds(msTimer_));
            // timer_->expires_from_now(boost::asio::chrono::milliseconds(timeDurationMs_));

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

    };

} // OwlTagProcessor

#endif //OWLAPRILTAGPROCESSOR_TAGPROCESSOR_H
