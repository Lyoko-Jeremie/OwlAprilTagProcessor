#include <iostream>
#include <string>
#include <memory>

#include "OwlLog/OwlLog.h"

//#include "3thlib/cpp-httplib/httplib.h"

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/thread.hpp>
#include <boost/beast.hpp>
#include <boost/program_options.hpp>

#include "AprilTagData/AprilTagData.h"
#include "GetImage/GetImage.h"
#include "SendResult/SendResult.h"
#include "ConfigLoader/TagConfigLoader.h"
#include "TagProcessor/TagProcessor.h"


#ifndef DEFAULT_CONFIG
#define DEFAULT_CONFIG R"(config.json)"
#endif // DEFAULT_CONFIG


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

extern "C" {
#include "apriltag.h"
#include "tag36h11.h"
#include "tag25h9.h"
#include "tag16h5.h"
#include "tagCircle21h7.h"
#include "tagCircle49h12.h"
#include "tagCustom48h12.h"
#include "tagStandard41h12.h"
#include "tagStandard52h13.h"
#include "common/getopt.h"
}

int main1(int argc, const char *argv[]) {
    std::cout << "Hello, World!" << std::endl;
    OwlLog::threadName = "main";
    OwlLog::init_logging();

//    image_u8_t *im = image_u8_create_from_pnm("../1.jpg");
//
//    if (!im) {
//        std::cout << "(!im)" << std::endl;
//        return 0;
//    }

    auto image = cv::imread("../1.jpg");

    if (image.empty()) {
        std::cout << "(image.empty())" << std::endl;
        return 0;
    }

    BOOST_LOG_TRIVIAL(trace) << "image:"
                             << " cols " << image.cols
                             << " rows " << image.rows;
    if (image.channels() > 1) {
        cv::cvtColor(image, image, cv::ColorConversionCodes::COLOR_BGR2GRAY);
    }
    cv::resize(image, image, cv::Size{640, 480}, 0, 0, cv::InterpolationFlags::INTER_CUBIC);

    image_u8_t im = {.width = image.cols,
            .height = image.rows,
            .stride = image.cols,
            .buf = image.data
    };

    BOOST_LOG_TRIVIAL(trace) << "image:"
                             << " cols " << im.width
                             << " rows " << im.height
                             << " stride " << im.stride;

    apriltag_detector_t *td = apriltag_detector_create();
    apriltag_family_t *tf = tagStandard41h12_create();
    apriltag_detector_add_family(td, tf);
    zarray_t *detections = apriltag_detector_detect(td, &im);


    BOOST_LOG_TRIVIAL(trace) << "zarray_size(detections):" << zarray_size(detections);

    for (int i = 0; i < zarray_size(detections); i++) {
        apriltag_detection_t *det;
        zarray_get(detections, i, &det);

        // TODO Do stuff with detections here.
        BOOST_LOG_TRIVIAL(trace)
            << "calcTag : " << i
            << " id " << det->id
            << " c[0]x " << det->c[0]
            << " c[1]y " << det->c[1]
            << " p[0] (" << det->p[0][0] << "," << det->p[0][0] << ")"
            << " p[1] (" << det->p[1][0] << "," << det->p[1][0] << ")"
            << " p[2] (" << det->p[2][0] << "," << det->p[2][0] << ")"
            << " p[3] (" << det->p[3][0] << "," << det->p[3][0] << ")"
            << " hamming " << det->hamming
            << " decision_margin " << det->decision_margin;

        // Do stuff with detections here.
    }
    // Cleanup.
    tagStandard41h12_destroy(tf);
    apriltag_detector_destroy(td);

}

int main(int argc, const char *argv[]) {
    std::cout << "Hello, World!" << std::endl;
    OwlLog::threadName = "main";
    OwlLog::init_logging();



    // parse start params
    std::string config_file;
    boost::program_options::options_description desc("options");
    desc.add_options()
            ("config,c", boost::program_options::value<std::string>(&config_file)->
                    default_value(DEFAULT_CONFIG)->
                    value_name("CONFIG"), "specify config file")
            ("help,h", "print help message")
            ("version,v", "print version and build info");
    boost::program_options::positional_options_description pd;
    pd.add("config", 1);
    boost::program_options::variables_map vMap;
    boost::program_options::store(
            boost::program_options::command_line_parser(argc, argv)
                    .options(desc)
                    .positional(pd)
                    .run(), vMap);
    boost::program_options::notify(vMap);
    if (vMap.count("help")) {
        std::cout << "usage: " << argv[0] << " [[-c] CONFIG]" << "\n" << std::endl;

        std::cout << "    OwlAccessTerminal  Copyright (C) 2023 \n"
                  << "\n" << std::endl;

        std::cout << desc << std::endl;
        return 0;
    }
    if (vMap.count("version")) {
        std::cout << "Boost " << BOOST_LIB_VERSION <<
                  ", OpenCV " << CV_VERSION
                  << std::endl;
        return 0;
    }

    BOOST_LOG_TRIVIAL(info) << "config_file: " << config_file;

    // load config
    auto config = std::make_shared<OwlTagConfigLoader::TagConfigLoader>();
    config->init(config_file);
    config->print();


    boost::asio::io_context ioc_Processor;
    auto ptr_GetImage = std::make_shared<OwlGetImage::GetImage>(ioc_Processor);
    auto ptr_SendResult = std::make_shared<OwlSendResult::SendResult>(ioc_Processor);
    auto ptr_AprilTagData = std::make_shared<OwlAprilTagData::AprilTagData>(config);
    auto ptr_TagProcessor = std::make_shared<OwlTagProcessor::TagProcessor>(
            ioc_Processor,
            ptr_GetImage,
            ptr_SendResult,
            ptr_AprilTagData,
            config
    );
    ptr_TagProcessor->start();

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
                ioc_Processor.stop();
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
    tg.create_thread(ThreadCallee{ioc_Processor, tg, "ioc_Processor"});
    tg.create_thread(ThreadCallee{ioc_keyboard, tg, "ioc_keyboard"});


    BOOST_LOG_TRIVIAL(info) << "boost::thread_group running";
    BOOST_LOG_TRIVIAL(info) << "boost::thread_group size : " << tg.size();
    tg.join_all();
    BOOST_LOG_TRIVIAL(info) << "boost::thread_group end";

    BOOST_LOG_TRIVIAL(info) << "boost::thread_group all clear.";
    return 0;
}
