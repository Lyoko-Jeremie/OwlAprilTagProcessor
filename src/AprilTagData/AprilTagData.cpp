// jeremie

#include "AprilTagData.h"
#include <boost/core/ignore_unused.hpp>
#include <boost/log/trivial.hpp>
#include <utility>
#include <cmath>
#include <limits>


#ifdef USE_AprilTagDataOpenCVImpl
#include <opencv2/aruco.hpp>
#else
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
#endif // USE_AprilTagDataOpenCVImpl


namespace OwlAprilTagData {

    AprilTagData::AprilTagData(
            const std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> &tagConfigLoader
    ) : impl(std::make_shared<AprilTagDataImpl>(tagConfigLoader)) {}

#ifdef USE_AprilTagDataOpenCVImpl
    struct AprilTagDataOpenCVImpl : public std::enable_shared_from_this<AprilTagDataImpl> {
        explicit AprilTagDataOpenCVImpl(std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> tagConfigLoader)
                : tagConfigLoader_(std::move(tagConfigLoader)),
                  detectorParams_(),
                  dictionary_(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_APRILTAG_36h11)),
                  detector_(dictionary_, detectorParams_) {
        }

        std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> tagConfigLoader_;

        cv::aruco::DetectorParameters detectorParams_;
        cv::aruco::Dictionary dictionary_;
        cv::aruco::ArucoDetector detector_;


        auto analysis(cv::Mat image) -> std::shared_ptr<AprilTagDataObject> {

            auto data_r = std::make_shared<AprilTagDataObject>();

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


            std::vector<int> markerIds;
            std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
//            detector_.detectMarkers(image, markerCorners, markerIds, rejectedCandidates);
            detector_.detectMarkers(image, markerCorners, markerIds);

//            return data_r;

//            BOOST_LOG_TRIVIAL(trace) << "markerIds.size():" << markerIds.size();
//            BOOST_LOG_TRIVIAL(trace) << "markerCorners.size():" << markerCorners.size();
//            BOOST_LOG_TRIVIAL(trace) << "rejectedCandidates.size():" << rejectedCandidates.size();


            data_r->tagInfo.reserve(markerIds.size());


            for (size_t i = 0; i < markerIds.size(); i++) {

                auto pp = markerCorners.at(i);
                cv::Point2f c{
                        (pp.at(0).x + pp.at(1).x + pp.at(2).x + pp.at(3).x) / 4.f,
                        (pp.at(0).y + pp.at(1).y + pp.at(2).y + pp.at(3).y) / 4.f,
                };
                BOOST_LOG_TRIVIAL(trace)
                    << "calcTag : " << i
                    << " id " << markerIds.at(i)
                    << " c[0]x " << c.x
                    << " c[1]y " << c.y
                    << " p[0] (" << pp.at(0).x << "," << pp.at(0).y << ")"
                    << " p[1] (" << pp.at(1).x << "," << pp.at(1).y << ")"
                    << " p[2] (" << pp.at(2).x << "," << pp.at(2).y << ")"
                    << " p[3] (" << pp.at(3).x << "," << pp.at(3).y << ")";

                data_r->tagInfo.emplace_back(
                        AprilTagDataTagInfo{
                                .id=markerIds.at(i),
                                .hamming=0,
                                .decision_margin=0,

                                .centerX=c.x,
                                .centerY=c.y,

                                .cornerLTx=pp.at(0).x,
                                .cornerLTy=pp.at(0).y,

                                .cornerRTx=pp.at(1).x,
                                .cornerRTy=pp.at(1).y,

                                .cornerRBx=pp.at(2).x,
                                .cornerRBy=pp.at(2).y,

                                .cornerLBx=pp.at(3).x,
                                .cornerLBy=pp.at(3).y,
                        }
                );

            }

            if (!data_r->tagInfo.empty()) {
                // find center tag here
                size_t miniIndex = 0;
                double miniDistance = std::numeric_limits<double>::max();
                double imageX = image.cols / 2.;
                double imageY = image.rows / 1.;
                for (size_t i = 0; i < data_r->tagInfo.size(); ++i) {
                    auto p = data_r->tagInfo.at(i);
                    double d = std::pow(p.centerX - imageX, 2) + std::pow(p.centerY - imageY, 2);
                    if (miniDistance > d) {
                        miniDistance = d;
                        miniIndex = i;
                    }
                }
                data_r->center = std::make_shared<AprilTagDataTagInfo>(data_r->tagInfo.at(miniIndex));
                BOOST_LOG_TRIVIAL(trace)
                    << "centerTag : "
                    << " id " << data_r->center->id
                    << " c[0]x " << data_r->center->centerX
                    << " c[1]y " << data_r->center->centerY
                    << " p[0] (" << data_r->center->cornerLTx << "," << data_r->center->cornerLTy << ")"
                    << " p[1] (" << data_r->center->cornerRTx << "," << data_r->center->cornerRTy << ")"
                    << " p[2] (" << data_r->center->cornerRBx << "," << data_r->center->cornerRBy << ")"
                    << " p[3] (" << data_r->center->cornerLBx << "," << data_r->center->cornerLBy << ")";
            } else {
                data_r->center = nullptr;
            }

            return data_r;
        }
    };
#else

    struct AprilTagDataOriginImpl : public std::enable_shared_from_this<AprilTagDataImpl> {
        explicit AprilTagDataOriginImpl(std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> tagConfigLoader)
                : tagConfigLoader_(std::move(tagConfigLoader)) {
            td = apriltag_detector_create();
            tf = tag36h11_create();
            // bits_corrected : 1 use 1MB , 2 use 193MB, 3 use 1.3GB
            apriltag_detector_add_family_bits(
                    td, tf,
                    2
//                    tagConfigLoader_->config.configAprilTagData.aprilTagDetectorMaxHammingBitsCorrected
            );
        }

        std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> tagConfigLoader_;

        auto analysis(cv::Mat image) -> std::shared_ptr<AprilTagDataObject> {

//            BOOST_LOG_TRIVIAL(trace) << "image:"
//                                     << " isContinuous " << image.isContinuous()
//                                     << " cols " << image.cols
//                                     << " rows " << image.rows;

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

//            BOOST_LOG_TRIVIAL(trace) << "image:"
//                                     << " isContinuous " << image.isContinuous()
//                                     << " cols " << image.cols
//                                     << " rows " << image.rows;

            image_u8_t img_header = {.width = image.cols,
                    .height = image.rows,
                    .stride = image.cols,
                    .buf = image.data
            };

//            BOOST_LOG_TRIVIAL(trace) << "image:"
//                                     << " cols " << img_header.width
//                                     << " rows " << img_header.height
//                                     << " stride " << img_header.stride;

            zarray_t *detections = apriltag_detector_detect(td, &img_header);

            auto data_r = std::make_shared<AprilTagDataObject>();

            data_r->imageX = image.cols;
            data_r->imageY = image.rows;

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

            if (!data_r->tagInfo.empty()) {
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

        ~AprilTagDataOriginImpl() {
            // Cleanup.
            tagStandard41h12_destroy(tf);
            apriltag_detector_destroy(td);
        }

        apriltag_detector_t *td;
        apriltag_family_t *tf;
    };

#endif // USE_AprilTagDataOpenCVImpl

    auto AprilTagDataObject2ResultType(std::shared_ptr<AprilTagDataObject> o)
    -> std::shared_ptr<AprilTagResultType> {
        std::shared_ptr<AprilTagResultType> m = std::make_shared<AprilTagResultType>();
        boost::json::array tagList;
        for (auto &a: o->tagInfo) {
            tagList.emplace_back(a.to_json_value());
        }
        boost::json::object v{
                {"tagList",      tagList},
                {"imageX", o->imageX},
                {"imageY", o->imageY},
        };
        if (o->center) {
            v.emplace("centerTag", o->center->to_json_value());
//            m->params.insert({"center", boost::json::serialize(o->center->to_json_value())});
        } else {
//            m->params.insert({"center", nullptr});
        }
        m->body = boost::json::serialize(v);
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

