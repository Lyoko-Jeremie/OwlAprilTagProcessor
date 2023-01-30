// jeremie

#include "TagConfigLoader.h"

namespace OwlTagConfigLoader {

    // https://www.boost.org/doc/libs/1_81_0/libs/json/doc/html/json/examples.html
    void pretty_print(std::ostream &os, boost::json::value const &jv, std::string *indent = nullptr) {
        std::string indent_;
        if (!indent)
            indent = &indent_;
        switch (jv.kind()) {
            case boost::json::kind::object: {
                os << "{\n";
                indent->append(4, ' ');
                auto const &obj = jv.get_object();
                if (!obj.empty()) {
                    auto it = obj.begin();
                    for (;;) {
                        os << *indent << boost::json::serialize(it->key()) << " : ";
                        pretty_print(os, it->value(), indent);
                        if (++it == obj.end())
                            break;
                        os << ",\n";
                    }
                }
                os << "\n";
                indent->resize(indent->size() - 4);
                os << *indent << "}";
                break;
            }

            case boost::json::kind::array: {
                os << "[\n";
                indent->append(4, ' ');
                auto const &arr = jv.get_array();
                if (!arr.empty()) {
                    auto it = arr.begin();
                    for (;;) {
                        os << *indent;
                        pretty_print(os, *it, indent);
                        if (++it == arr.end())
                            break;
                        os << ",\n";
                    }
                }
                os << "\n";
                indent->resize(indent->size() - 4);
                os << *indent << "]";
                break;
            }

            case boost::json::kind::string: {
                os << boost::json::serialize(jv.get_string());
                break;
            }

            case boost::json::kind::uint64:
                os << jv.get_uint64();
                break;

            case boost::json::kind::int64:
                os << jv.get_int64();
                break;

            case boost::json::kind::double_:
                os << jv.get_double();
                break;

            case boost::json::kind::bool_:
                if (jv.get_bool())
                    os << "true";
                else
                    os << "false";
                break;

            case boost::json::kind::null:
                os << "null";
                break;
        }

        if (indent->empty())
            os << "\n";
    }

    boost::json::value TagConfigLoader::load_json_file(const std::string &filePath) {
        boost::system::error_code ec;
        std::stringstream ss;
        boost::filesystem::ifstream f(filePath);
        if (!f) {
            BOOST_LOG_TRIVIAL(error) << "load_json_file (!f)";
            return nullptr;
        }
        ss << f.rdbuf();
        f.close();
        boost::json::stream_parser p;
        p.write(ss.str(), ec);
        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "load_json_file (ec) " << ec.what();
            return nullptr;
        }
        p.finish(ec);
        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "load_json_file (ec) " << ec.what();
            return nullptr;
        }
        return p.release();
    }

    Config TagConfigLoader::parse_json(const boost::json::value &&json_v) {
        const auto &root = json_v.as_object();

        {
            std::stringstream ss;
            pretty_print(ss, root);
            auto s = ss.str();
            BOOST_LOG_TRIVIAL(info) << "parse_json from : \n" << s;
            // BOOST_LOG_TRIVIAL(info) << "parse_json from : \n" << boost::json::serialize(root);
        }

        Config config_;

        config_.timeStartMs = get(root, "timeStartMs", config_.timeStartMs);
        config_.timeDurationMs = get(root, "timeDurationMs", config_.timeDurationMs);

        if (root.contains("configGetImage")) {
            auto configGetImage = getObj(root, "configGetImage");
            config_.configGetImage.host = get(configGetImage, "host", config_.configGetImage.host);
            config_.configGetImage.port = get(configGetImage, "port",
                                              config_.configGetImage.port);
            config_.configGetImage.target = get(configGetImage, "target",
                                                config_.configGetImage.target);
            config_.configGetImage.version = get(configGetImage, "version",
                                                 config_.configGetImage.version);
            config_.configGetImage.timeoutMs = get(configGetImage, "timeoutMs",
                                                   config_.configGetImage.timeoutMs);
        }

        if (root.contains("configSendResult")) {
            auto configSendResult = getObj(root, "configSendResult");
            config_.configSendResult.host = get(configSendResult, "host", config_.configSendResult.host);
            config_.configSendResult.port = get(configSendResult, "port",
                                                config_.configSendResult.port);
            config_.configSendResult.target = get(configSendResult, "target",
                                                  config_.configSendResult.target);
            config_.configSendResult.version = get(configSendResult, "version",
                                                   config_.configSendResult.version);
            config_.configSendResult.timeoutMs = get(configSendResult, "timeoutMs",
                                                     config_.configSendResult.timeoutMs);
        }

        return config_;
    }

} // OwlTagConfigLoader