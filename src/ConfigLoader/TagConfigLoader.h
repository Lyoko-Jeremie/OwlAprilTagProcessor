// jeremie

#ifndef OWLAPRILTAGPROCESSOR_TAGCONFIGLOADER_H
#define OWLAPRILTAGPROCESSOR_TAGCONFIGLOADER_H

#include <memory>
#include <sstream>
#include <variant>
#include <functional>
#include <map>
#include <tuple>
#include <boost/filesystem/fstream.hpp>
#include <boost/system.hpp>
#include <boost/json.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/log/trivial.hpp>

namespace OwlTagConfigLoader {

    struct ConfigGetImage {

        std::string host = "127.0.0.1";
        std::string port = "23331";
        std::string target = "/1";
        int version = 11;
        long timeoutMs = 1 * 1000;
    };

    struct ConfigSendResult {

        std::string host = "127.0.0.1";
        std::string port = "23338";
        std::string target = "/cmd";
        int version = 11;
        long timeoutMs = 1 * 1000;
    };

    struct ConfigTagProcessor {
        long timeStartMs = 1 * 1000;
        long timeDurationMs = 300;
        long timeoutCountLimit = 6;
        long timeDurationMini = 30;
    };

    struct ConfigAprilTagData {
        int resizeWidth = 640;
        int resizeHeight = 480;
        int aprilTagDetectorMaxHammingBitsCorrected = 1;
    };

    struct Config {
        ConfigTagProcessor configTagProcessor;
        ConfigAprilTagData configAprilTagData;
        ConfigGetImage configGetImage;
        ConfigSendResult configSendResult;
    };


    class TagConfigLoader : public std::enable_shared_from_this<TagConfigLoader> {
    public:

        Config config;

        void print() {
            BOOST_LOG_TRIVIAL(info)
                << "\n"
                << "\n" << "TagConfigLoader config:"
                << "\n" << "ConfigTagProcessor :"
                << "\n" << "\t timeStartMs " << config.configTagProcessor.timeStartMs
                << "\n" << "\t timeDurationMs " << config.configTagProcessor.timeDurationMs
                << "\n" << "\t timeoutCountLimit " << config.configTagProcessor.timeoutCountLimit
                << "\n" << "\t timeDurationMini " << config.configTagProcessor.timeDurationMini
                << "\n" << "ConfigAprilTagData :"
                << "\n" << "\t resizeWidth " << config.configAprilTagData.resizeWidth
                << "\n" << "\t resizeHeight " << config.configAprilTagData.resizeHeight
                << "\n" << "\t aprilTagDetectorMaxHammingBitsCorrected "
                << config.configAprilTagData.aprilTagDetectorMaxHammingBitsCorrected
                << "\n" << "ConfigGetImage :"
                << "\n" << "\t host " << config.configGetImage.host
                << "\n" << "\t port " << config.configGetImage.port
                << "\n" << "\t target " << config.configGetImage.target
                << "\n" << "\t version " << config.configGetImage.version
                << "\n" << "\t timeoutMs " << config.configGetImage.timeoutMs
                << "\n" << "ConfigSendResult :"
                << "\n" << "\t host " << config.configSendResult.host
                << "\n" << "\t port " << config.configSendResult.port
                << "\n" << "\t target " << config.configSendResult.target
                << "\n" << "\t version " << config.configSendResult.version
                << "\n" << "\t timeoutMs " << config.configSendResult.timeoutMs
                << "";
        }

        void init(const std::string &filePath) {
            auto j = load_json_file(filePath);
            BOOST_LOG_TRIVIAL(info) << "j.is_object() " << j.is_object() << "\t"
                                    << "j.kind() " << boost::json::to_string(j.kind());
            if (j.is_object()) {
                config = parse_json(j.as_object());
            } else {
                BOOST_LOG_TRIVIAL(error)
                    << "TagConfigLoader: config file not exit OR cannot load config file OR config file invalid.";
            }
        }

        static boost::json::value load_json_file(const std::string &filePath);

        Config parse_json(const boost::json::value &&json_v);

        template<typename T>
        std::remove_cvref_t<T> get(const boost::json::object &v, boost::string_view key, T &&d) {
            try {
                if (!v.contains(key)) {
                    return d;
                }
                auto rr = boost::json::try_value_to<std::remove_cvref_t<T>>(v.at(key));
                return rr.has_value() ? rr.value() : d;
            } catch (std::exception &e) {
                return d;
            }
        }

        boost::json::object getObj(const boost::json::object &v, boost::string_view key) {
            try {
                if (!v.contains(key)) {
                    return {};
                }
                auto oo = v.at(key);
                return oo.as_object();
            } catch (std::exception &e) {
                return {};
            }
        }
    };

} // OwlTagConfigLoader

#endif //OWLAPRILTAGPROCESSOR_TAGCONFIGLOADER_H
