// jeremie

#include "AprilTagData.h"
#include <boost/core/ignore_unused.hpp>
#include <boost/log/trivial.hpp>
#include <utility>
#include <cmath>
#include <limits>

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

namespace OwlAprilTagData {

    AprilTagData::AprilTagData(
            const std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> &tagConfigLoader
    ) : impl(std::make_shared<AprilTagDataImpl>(tagConfigLoader)) {}

    struct AprilTagDataImpl : public std::enable_shared_from_this<AprilTagDataImpl> {
        explicit AprilTagDataImpl(std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> tagConfigLoader)
                : tagConfigLoader_(std::move(tagConfigLoader)) {
            td = apriltag_detector_create();
            tf = tagStandard41h12_create();
            // bits_corrected : 1 use 1MB , 2 use 193MB, 3 use 1.3GB
            apriltag_detector_add_family_bits(
                    td, tf,
                    tagConfigLoader_->config.configAprilTagData.aprilTagDetectorMaxHammingBitsCorrected);
        }

        std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> tagConfigLoader_;

        auto analysis(cv::Mat image) -> std::shared_ptr<AprilTagDataObject> {

            BOOST_LOG_TRIVIAL(trace) << "image:"
                                     << " isContinuous " << image.isContinuous()
                                     << " cols " << image.cols
                                     << " rows " << image.rows;

            if (image.channels() > 1) {
                cv::cvtColor(image, image, cv::ColorConversionCodes::COLOR_BGR2GRAY);
            }
            if (image.cols > tagConfigLoader_->config.configAprilTagData.resizeWidth
                || image.rows > tagConfigLoader_->config.configAprilTagData.resizeHeight) {
                cv::resize(image, image, cv::Size{
                        tagConfigLoader_->config.configAprilTagData.resizeWidth,
                        tagConfigLoader_->config.configAprilTagData.resizeHeight
                }, 0, 0, cv::InterpolationFlags::INTER_CUBIC);
            }

            BOOST_LOG_TRIVIAL(trace) << "image:"
                                     << " isContinuous " << image.isContinuous()
                                     << " cols " << image.cols
                                     << " rows " << image.rows;

            image_u8_t img_header = {.width = image.cols,
                    .height = image.rows,
                    .stride = image.cols,
                    .buf = image.data
            };

            BOOST_LOG_TRIVIAL(trace) << "image:"
                                     << " cols " << img_header.width
                                     << " rows " << img_header.height
                                     << " stride " << img_header.stride;

            zarray_t *detections = apriltag_detector_detect(td, &img_header);

            auto data_r = std::make_shared<AprilTagDataObject>();
            data_r->tagInfo.reserve(zarray_size(detections));

            for (int i = 0; i < zarray_size(detections); i++) {
                apriltag_detection_t *det;
                zarray_get(detections, i, &det);

                BOOST_LOG_TRIVIAL(trace)
                    << "calcTag : " << i
                    << " id " << det->id
                    << " c[0]x " << det->c[0]
                    << " c[1]y " << det->c[1]
                    << " p[0] (" << det->p[0][0] << "," << det->p[0][1] << ")"
                    << " p[1] (" << det->p[1][0] << "," << det->p[1][1] << ")"
                    << " p[2] (" << det->p[2][0] << "," << det->p[2][1] << ")"
                    << " p[3] (" << det->p[3][0] << "," << det->p[3][1] << ")"
                    << " hamming " << det->hamming
                    << " decision_margin " << det->decision_margin;

                data_r->tagInfo.emplace_back(
                        AprilTagDataTagInfo{
                                .id=det->id,
                                .hamming=det->hamming,
                                .decision_margin=det->decision_margin,

                                .centerX=det->c[0],
                                .centerY=det->c[1],

                                .cornerLTx=det->p[0][0],
                                .cornerLTy=det->p[0][1],

                                .cornerRTx=det->p[1][0],
                                .cornerRTy=det->p[1][1],

                                .cornerRBx=det->p[2][0],
                                .cornerRBy=det->p[2][1],

                                .cornerLBx=det->p[3][0],
                                .cornerLBy=det->p[3][1],
                        }
                );

            }

            if (data_r->tagInfo.size() > 0) {
                // find center tag here
                size_t miniIndex = 0;
                double miniDistance = std::numeric_limits<double>::max();
                double imageCenterX = image.cols / 2.;
                double imageCenterY = image.rows / 1.;
                for (size_t i = 0; i < data_r->tagInfo.size(); ++i) {
                    auto p = data_r->tagInfo.at(i);
                    double d = std::pow(p.centerX - imageCenterX, 2) + std::pow(p.centerY - imageCenterY, 2);
                    if (miniDistance > d) {
                        miniDistance = d;
                        miniIndex = i;
                    }
                }
                data_r->center = std::make_shared<AprilTagDataTagInfo>(data_r->tagInfo.at(miniIndex));
            } else {
                data_r->center = nullptr;
            }

            return data_r;
        }

        ~AprilTagDataImpl() {
            // Cleanup.
            tagStandard41h12_destroy(tf);
            apriltag_detector_destroy(td);
        }

        apriltag_detector_t *td;
        apriltag_family_t *tf;
    };

    auto AprilTagDataObject2ResultType(std::shared_ptr<AprilTagDataObject> o)
    -> std::shared_ptr<AprilTagResultType> {
        std::shared_ptr<AprilTagResultType> m = std::make_shared<AprilTagResultType>();
        boost::json::array tagList;
        for (auto &a: o->tagInfo) {
            tagList.emplace_back(a.to_json_value());
        }
        boost::json::value v{
                {"tagList", tagList},
        };
        m->body = boost::json::serialize(v);
        if (o->center) {
            m->params.insert({"center", boost::json::serialize(o->center->to_json_value())});
        } else {
//            m->params.insert({"center", nullptr});
        }
        // m->params = decltype(m->params){
        //         decltype(m->params)::value_type{"center", boost::json::serialize((o->center.to_json_value())}
        // };
        return m;
    }

    std::shared_ptr<AprilTagDataObject> AprilTagData::analysis(cv::Mat image) {
        return impl->analysis(std::move(image));
    }

    boost::json::value AprilTagDataTagInfo::to_json_value() {
        return {
                {"id",   id},
                {"ham",  hamming},
                {"dm",   decision_margin},
                {"cX",   centerX},
                {"cY",   centerY},
                {"cLTx", cornerLTx},
                {"cLTy", cornerLTy},
                {"cRTx", cornerRTx},
                {"cRTy", cornerRTy},
                {"cRBx", cornerRBx},
                {"cRBy", cornerRBy},
                {"cLBx", cornerLBx},
                {"cLBy", cornerLBy},
        };
    }
} // OwlAprilTagData

