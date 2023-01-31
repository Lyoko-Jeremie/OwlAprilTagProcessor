// jeremie

#ifndef OWLAPRILTAGPROCESSOR_SENDRESULT_H
#define OWLAPRILTAGPROCESSOR_SENDRESULT_H

#include <string>
#include <memory>
#include <map>

#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "../AprilTagData/AprilTagData.h"

namespace OwlSendResult {

    class SendResultSession;

    using CallbackFunctionType = std::function<void(boost::beast::error_code ec, bool ok)>;

    class SendResult {
    public:
        explicit
        SendResult(boost::asio::io_context &ioc)
                : ioc_(ioc) {
        }

    private:
        boost::asio::io_context &ioc_;

    public:

        long timeoutMs = 1 * 1000;

        void test(
                const std::string &host,
                const std::string &port,
                const std::string &target,
                int version);

        std::shared_ptr<SendResultSession> send(
                const std::string &host,
                const std::string &port,
                const std::string &target,
                int version,
                std::shared_ptr<OwlAprilTagData::AprilTagResultType> data,
                CallbackFunctionType &&callback);

    };

} // OwlSendResult

#endif //OWLAPRILTAGPROCESSOR_SENDRESULT_H
