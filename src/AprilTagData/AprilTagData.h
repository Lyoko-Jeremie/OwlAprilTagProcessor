// jeremie

#ifndef OWLAPRILTAGPROCESSOR_APRILTAGDATA_H
#define OWLAPRILTAGPROCESSOR_APRILTAGDATA_H

#include <memory>
#include <opencv2/opencv.hpp>
#include <boost/json.hpp>
#include "../ConfigLoader/TagConfigLoader.h"

namespace OwlAprilTagData {

    struct AprilTagDataOriginImpl;
    struct AprilTagDataOpenCVImpl;

    using AprilTagDataImpl = AprilTagDataOriginImpl;
//    using AprilTagDataImpl = AprilTagDataOpenCVImpl;

    struct AprilTagDataTagInfo {
        // from :
        //      typedef struct apriltag_detection apriltag_detection_t;
        //      struct apriltag_detection
        int id;
        int hamming;
        float decision_margin;
        double centerX;
        double centerY;
        double cornerLTx;
        double cornerLTy;
        double cornerRTx;
        double cornerRTy;
        double cornerRBx;
        double cornerRBy;
        double cornerLBx;
        double cornerLBy;

        boost::json::value to_json_value();
    };

    struct AprilTagDataObject {
        std::vector<AprilTagDataTagInfo> tagInfo;
        std::shared_ptr<AprilTagDataTagInfo> center;
    };

    struct AprilTagResultType {
        std::map<std::string, std::string> params;
        std::string body;
    };

    auto AprilTagDataObject2ResultType(std::shared_ptr<AprilTagDataObject> o) -> std::shared_ptr<AprilTagResultType>;

    class AprilTagData : public std::enable_shared_from_this<AprilTagData> {
    public:
        explicit AprilTagData(const std::shared_ptr<OwlTagConfigLoader::TagConfigLoader> &TagConfigLoader);

        std::shared_ptr<AprilTagDataObject> analysis(cv::Mat image);

    private:
        std::shared_ptr<AprilTagDataImpl> impl;

    };

} // OwlAprilTagData

#endif //OWLAPRILTAGPROCESSOR_APRILTAGDATA_H
