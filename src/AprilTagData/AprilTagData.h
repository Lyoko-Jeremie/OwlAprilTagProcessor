// jeremie

#ifndef OWLAPRILTAGPROCESSOR_APRILTAGDATA_H
#define OWLAPRILTAGPROCESSOR_APRILTAGDATA_H

#include <memory>
#include <opencv2/opencv.hpp>

namespace OwlAprilTagData {

    struct AprilTagDataImpl;

    class AprilTagData : public std::enable_shared_from_this<AprilTagData> {
    public:
        AprilTagData() = default;

    private:
        std::shared_ptr<AprilTagDataImpl> impl{};

    };

} // OwlAprilTagData

#endif //OWLAPRILTAGPROCESSOR_APRILTAGDATA_H
