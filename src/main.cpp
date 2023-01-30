#include <iostream>
#include <string>
#include <memory>

#include "OwlLog/OwlLog.h"

//#include "3thlib/cpp-httplib/httplib.h"

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/thread.hpp>
#include <boost/beast.hpp>

#include "GetImage/GetImage.h"

struct ThreadCallee {
    boost::asio::io_context &ioc;
    boost::thread_group &tg;
    std::string thisThreadName;

    int operator()() {
        try {
            OwlLog::threadName = thisThreadName;
            BOOST_LOG_TRIVIAL(info) << ">>>" << OwlLog::threadName << "<<< running thread <<< <<<";
            // use work to keep ioc run
            auto work_guard_ = boost::asio::make_work_guard(ioc);
            ioc.run();
            boost::ignore_unused(work_guard_);
            BOOST_LOG_TRIVIAL(warning) << "ThreadCallee ioc exit. thread: " << OwlLog::threadName;
        } catch (int e) {
            tg.interrupt_all();
            BOOST_LOG_TRIVIAL(error) << "catch (int) exception: " << e;
            return -1;
        } catch (const std::exception &e) {
            tg.interrupt_all();
            BOOST_LOG_TRIVIAL(error) << "catch std::exception: " << e.what();
            return -1;
        } catch (...) {
            tg.interrupt_all();
            BOOST_LOG_TRIVIAL(error) << "catch (...) exception";
            return -1;
        }
        return 0;
    }
};


int main() {
    std::cout << "Hello, World!" << std::endl;
    OwlLog::threadName = "main";
    OwlLog::init_logging();

//    httplib::Client cli("bing.com", 80);
//
////    cli.set_connection_timeout(0, 300000); // 300 milliseconds
////    cli.set_read_timeout(5, 0); // 5 seconds
////    cli.set_write_timeout(5, 0); // 5 seconds
//
//    std::cout << "res 1";
//    auto res = cli.Get("/1");
//
//    std::cout << "res 2";
//
//    if (res) {
//        if (res->status == 200) {
//            std::cout << res->body << std::endl;
//        }
//        std::cout << "res->status: " << res->status << std::endl;
//    } else {
//        auto err = res.error();
//        std::cout << "HTTP error: " << httplib::to_string(err) << std::endl;
//    }


    boost::asio::io_context ioc;
    auto ppp = std::make_shared<OwlGetImage::GetImage>(ioc);

    std::string host{"bing.com"};
    std::string port{"80"};
    std::string target{""};
    int version = 11;

    ppp->test(host, port, target, version);

//    // The io_context is required for all I/O
//    boost::asio::io_context ioc;
//
//    // These objects perform our I/O
//    boost::asio::ip::tcp::resolver resolver(ioc);
//    boost::beast::tcp_stream stream(ioc);
//
//    // Look up the domain name
//    boost::beast::error_code ec1;
//    auto const results = resolver.resolve(host, port, ec1);
//    if (ec1) {
//        throw boost::beast::system_error{ec1};
//    }
//
//
//    // Make the connection on the IP address we get from a lookup
//    stream.connect(results);
//
//    // Set up an HTTP GET request message
//    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, target, version};
//    req.set(boost::beast::http::field::host, host);
//    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
//
//    // Send the HTTP request to the remote host
//    boost::beast::http::write(stream, req);
//
//    // This buffer is used for reading and must be persisted
//    boost::beast::flat_buffer buffer;
//
//    // Declare a container to hold the response
//    boost::beast::http::response<boost::beast::http::dynamic_body> res;
//
//    // Receive the HTTP response
//    boost::beast::http::read(stream, buffer, res);
//
//    // Write the message to standard out
//    std::cout << res << std::endl;
//
//    // Gracefully close the socket
//    boost::beast::error_code ec;
//    stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
//
//    // not_connected happens sometimes
//    // so don't bother reporting it.
//    //
//    if (ec && ec != boost::beast::errc::not_connected) {
//        throw boost::beast::system_error{ec};
//    }


    boost::asio::io_context ioc_keyboard;
    boost::asio::signal_set sig(ioc_keyboard);
    sig.add(SIGINT);
    sig.add(SIGTERM);
    sig.async_wait([&](const boost::system::error_code error, int signum) {
        if (error) {
            BOOST_LOG_TRIVIAL(error) << "got signal error: " << error.what() << " signum " << signum;
            return;
        }
        BOOST_LOG_TRIVIAL(error) << "got signal: " << signum;
        switch (signum) {
            case SIGINT:
            case SIGTERM: {
                // stop all service on there
                BOOST_LOG_TRIVIAL(info) << "stopping all service. ";
//                ioc_cmd.stop();
//                ioc_imageWeb.stop();
//                ioc_cameraReader.stop();
//                ioc_web_static.stop();
                ioc.stop();
                ioc_keyboard.stop();
            }
                break;
            default:
                BOOST_LOG_TRIVIAL(warning) << "sig switch default.";
                break;
        }
    });

    size_t processor_count = boost::thread::hardware_concurrency();
    BOOST_LOG_TRIVIAL(info) << "processor_count: " << processor_count;

    boost::thread_group tg;
//    tg.create_thread(ThreadCallee{ioc_cmd, tg, "ioc_cmd"});
//    tg.create_thread(ThreadCallee{ioc_imageWeb, tg, "ioc_imageWeb"});
//    tg.create_thread(ThreadCallee{ioc_cameraReader, tg, "ioc_cameraReader 1"});
//    tg.create_thread(ThreadCallee{ioc_cameraReader, tg, "ioc_cameraReader 2"});
//    tg.create_thread(ThreadCallee{ioc_web_static, tg, "ioc_web_static"});
    tg.create_thread(ThreadCallee{ioc, tg, "ioc_ioc"});
    tg.create_thread(ThreadCallee{ioc_keyboard, tg, "ioc_keyboard"});


    BOOST_LOG_TRIVIAL(info) << "boost::thread_group running";
    BOOST_LOG_TRIVIAL(info) << "boost::thread_group size : " << tg.size();
    tg.join_all();
    BOOST_LOG_TRIVIAL(info) << "boost::thread_group end";

    BOOST_LOG_TRIVIAL(info) << "boost::thread_group all clear.";
    return 0;
}
