// jeremie

#ifndef OWLAPRILTAGPROCESSOR_TAGPROCESSOR_H
#define OWLAPRILTAGPROCESSOR_TAGPROCESSOR_H

#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

namespace OwlTagProcessor {

    class GetImage;

    class SendResult;

    class AprilTagData;

    class TagProcessor : public std::enable_shared_from_this<TagProcessor> {
    public:
        TagProcessor(
                boost::asio::io_context &ioc,
                std::shared_ptr<GetImage> ptr_GetImage,
                std::shared_ptr<SendResult> ptr_SendResult,
                std::shared_ptr<AprilTagData> ptr_AprilTagData,
                long timeStartMs = 1000,
                long timeDurationMs = 300
        ) : ioc_(ioc),
            ptr_GetImage_(std::move(ptr_GetImage)),
            ptr_SendResult_(std::move(ptr_SendResult)),
            ptr_AprilTagData_(std::move(ptr_AprilTagData)),
            timeStartMs_(timeStartMs),
            timeDurationMs_(timeDurationMs) {

        }

    private:
        boost::asio::io_context &ioc_;

        std::shared_ptr<GetImage> ptr_GetImage_;
        std::shared_ptr<SendResult> ptr_SendResult_;
        std::shared_ptr<AprilTagData> ptr_AprilTagData_;

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
