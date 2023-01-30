#include <iostream>

#define WIN32_LEAN_AND_MEAN

#include "OwlLog.h"

#include "3thlib/cpp-httplib/httplib.h"
#include <Windows.h>

int main() {
    std::cout << "Hello, World!" << std::endl;
    OwlLog::init_logging();

    httplib::Client cli("127.0.0.1", 23331);

//    cli.set_connection_timeout(0, 300000); // 300 milliseconds
//    cli.set_read_timeout(5, 0); // 5 seconds
//    cli.set_write_timeout(5, 0); // 5 seconds

    std::cout << "res 1";
    auto res = cli.Get("/1");

    std::cout << "res 2";

    if (res) {
        if (res->status == 200) {
            std::cout << res->body << std::endl;
        }
        std::cout << "res->status: " << res->status << std::endl;
    } else {
        auto err = res.error();
        std::cout << "HTTP error: " << httplib::to_string(err) << std::endl;
    }

    return 0;
}
