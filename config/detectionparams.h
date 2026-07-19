#ifndef CONFIG_DETECTIONPARAMS_H
#define CONFIG_DETECTIONPARAMS_H

#include <QString>

struct ParamSetting {
    int value;
    int min;
    int max;
};

struct DetectionParams {
    ParamSetting sensitivity;
    ParamSetting edgeThreshold;
    ParamSetting minDistancePercent;
    ParamSetting minRadiusPercent;
    ParamSetting maxRadiusPercent;
};

struct ProcessingParams {
    int intervalMs;
    int blurKernelSize;
    double dp;
};

struct RenderParams {
    int circleColorB;
    int circleColorG;
    int circleColorR;
    int circleThickness;
};

struct CameraParams {
    int deviceIndex;
    QString fallbackVideoPath;
};

#endif
