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

    auto AprilTagDataObject2Map(std::shared_ptr<AprilTagDataObject> o) -> std::map<std::string, std::string> &&;

    class AprilTagData : public std::enable_shared_from_this<AprilTagData> {
    public:
        AprilTagData() = default;

        std::shared_ptr<AprilTagDataObject> analysis(cv::Mat image);

    private:
        std::shared_ptr<AprilTagDataImpl> impl{};

    };

} // OwlAprilTagData

#endif //OWLAPRILTAGPROCESSOR_APRILTAGDATA_H
