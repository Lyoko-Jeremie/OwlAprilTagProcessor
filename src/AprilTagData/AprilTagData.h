// jeremie

#ifndef OWLAPRILTAGPROCESSOR_APRILTAGDATA_H
#define OWLAPRILTAGPROCESSOR_APRILTAGDATA_H

#include <memory>
#include <opencv2/opencv.hpp>

namespace OwlAprilTagData {

    struct AprilTagDataImpl;

    // TODO
    struct AprilTagDataObject {
        // TODO
    };

    using AprilTagResultType = std::map<std::string, std::string>;

    auto AprilTagDataObject2ResultType(std::shared_ptr<AprilTagDataObject> o) -> std::shared_ptr<AprilTagResultType>;

    class AprilTagData : public std::enable_shared_from_this<AprilTagData> {
    public:
        AprilTagData();

        std::shared_ptr<AprilTagDataObject> analysis(cv::Mat image);

    private:
        std::shared_ptr<AprilTagDataImpl> impl;

    };

} // OwlAprilTagData

#endif //OWLAPRILTAGPROCESSOR_APRILTAGDATA_H
