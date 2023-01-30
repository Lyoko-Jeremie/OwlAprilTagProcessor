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

    class TagProcessor : public std::enable_shared_from_this<TagProcessor> {
    public:
        TagProcessor(
                boost::asio::io_context &ioc,
                std::shared_ptr<OwlGetImage::GetImage> ptr_GetImage,
                std::shared_ptr<OwlSendResult::SendResult> ptr_SendResult,
                std::shared_ptr<OwlAprilTagData::AprilTagData> ptr_AprilTagData,
                std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> TagConfigLoader
        ) : ioc_(ioc),
            ptr_GetImage_(std::move(ptr_GetImage)),
            ptr_SendResult_(std::move(ptr_SendResult)),
            ptr_AprilTagData_(std::move(ptr_AprilTagData)),
            ptr_TagConfigLoader_(std::move(TagConfigLoader)),
            timeStartMs_(ptr_TagConfigLoader_->config.timeStartMs),
            timeDurationMs_(ptr_TagConfigLoader_->config.timeDurationMs) {
        }

    private:
        boost::asio::io_context &ioc_;

        std::shared_ptr<OwlGetImage::GetImage> ptr_GetImage_;
        std::shared_ptr<OwlSendResult::SendResult> ptr_SendResult_;
        std::shared_ptr<OwlAprilTagData::AprilTagData> ptr_AprilTagData_;

        std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> ptr_TagConfigLoader_;

        long timeStartMs_ = 1000;
        long timeDurationMs_ = 300;

        std::shared_ptr<boost::asio::steady_timer> timer_;

    public:
        void start();

    private:

        void time_loop(const boost::system::error_code &ec);

    };

} // OwlTagProcessor

#endif //OWLAPRILTAGPROCESSOR_TAGPROCESSOR_H
