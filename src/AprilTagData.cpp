// jeremie

#include "AprilTagData.h"
#include <boost/core/ignore_unused.hpp>
#include <boost/log/trivial.hpp>

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

        void analysis(cv::Mat image) {
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

                {
                    // TODO this is debug
                    if (whenEnd) {
                        whenEnd();
                    }
                }
                return;
                // TODO calc tag pos here
                {
                    auto m = std::make_shared<OwlMailDefine::Cmd2Serial>();
                    m->callbackRunner = [whenEnd](OwlMailDefine::MailSerial2Cmd data) {
                        boost::ignore_unused(data);
                        // TODO
                        if (whenEnd) {
                            whenEnd();
                        }
                    };
                    sendMailCmd(std::move(m));
                }

            }
        }

        ~AprilTagDataImpl() {
            // Cleanup.
            tagStandard41h12_destroy(tf);
            apriltag_detector_destroy(td);
        }

        apriltag_detector_t *td;
        apriltag_family_t *tf;
    };

} // OwlAprilTagData

