// jeremie

#ifndef OWLAPRILTAGPROCESSOR_GETIMAGE_H
#define OWLAPRILTAGPROCESSOR_GETIMAGE_H

#include <string>
#include <memory>
#include <functional>

#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <opencv2/opencv.hpp>

namespace OwlGetImage {

    class session;

    using CallbackFunctionType = std::function<void(boost::beast::error_code ec, cv::Mat img)>;

    class GetImage {
    public:
        explicit
        GetImage(boost::asio::io_context &ioc)
                : ioc_(ioc) {
        }

        boost::asio::io_context &ioc_;

        void test(
                const std::string &host,
                const std::string &port,
                const std::string &target,
                int version);

        std::shared_ptr<session> get(
                const std::string &host,
                const std::string &port,
                const std::string &target,
                int version,
                CallbackFunctionType &&callback);
    };

} // OwlGetImage

#endif //OWLAPRILTAGPROCESSOR_GETIMAGE_H