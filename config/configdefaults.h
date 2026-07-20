#ifndef CONFIG_CONFIGDEFAULTS_H
#define CONFIG_CONFIGDEFAULTS_H

#include "config/detectionparams.h"

namespace config_defaults {

inline constexpr ParamSetting kSensitivity{35, 10, 100};
inline constexpr ParamSetting kEdgeThreshold{100, 50, 300};
inline constexpr ParamSetting kMinDistancePercent{33, 5, 100};
inline constexpr ParamSetting kMinRadiusPercent{10, 1, 100};
inline constexpr ParamSetting kMaxRadiusPercent{50, 1, 100};

inline DetectionParams detectionParams() {
    return {kSensitivity, kEdgeThreshold, kMinDistancePercent, kMinRadiusPercent, kMaxRadiusPercent};
}

inline constexpr ProcessingParams kProcessing{1000, 5, 1.0};
inline constexpr RenderParams kRender{0, 0, 255, 3};
inline constexpr int kCameraDeviceIndex = 0;

inline CameraParams cameraParams() {
    return {kCameraDeviceIndex, QStringLiteral("../test/test_video_01.mp4")};
}

}

#endif
