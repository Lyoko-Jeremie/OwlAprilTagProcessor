// jeremie

#include "AprilTagData.h"
#include <boost/core/ignore_unused.hpp>
#include <boost/log/trivial.hpp>
#include <utility>

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


    struct AprilTagDataImpl : public std::enable_shared_from_this<AprilTagDataImpl> {
        AprilTagDataImpl() {
            td = apriltag_detector_create();
            tf = tagStandard41h12_create();
            apriltag_detector_add_family(td, tf);
        }

        auto analysis(cv::Mat image) -> std::shared_ptr<AprilTagDataObject> {

            BOOST_LOG_TRIVIAL(trace) << "image:"
                                     << " cols " << image.cols
                                     << " rows " << image.rows;
            if (image.channels() > 1) {
                cv::cvtColor(image, image, cv::ColorConversionCodes::COLOR_BGR2GRAY);
            }
            cv::resize(image, image, cv::Size{640, 480}, 0, 0, cv::InterpolationFlags::INTER_CUBIC);

            image_u8_t img_header = {.width = image.cols,
                    .height = image.rows,
                    .stride = image.cols,
                    .buf = image.data
            };

            zarray_t *detections = apriltag_detector_detect(td, &img_header);

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

            }

            // TODO
            return std::make_shared<AprilTagDataObject>();
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
        // TODO
        m->try_emplace("", "");
        return m;
    }

    std::shared_ptr<AprilTagDataObject> AprilTagData::analysis(cv::Mat image) {
        return impl->analysis(std::move(image));
    }
} // OwlAprilTagData

