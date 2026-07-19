#include "config/appconfig.h"

#include <algorithm>

#include <QFileInfo>
#include <QSettings>
#include <QVariant>

#include "config/configdefaults.h"

namespace {
ParamSetting loadParam(QSettings& settings, const QString& group, const ParamSetting& fallback) {
    settings.beginGroup(group);
    bool minOk = false;
    bool maxOk = false;
    bool valueOk = false;
    int min = settings.value("min").toInt(&minOk);
    int max = settings.value("max").toInt(&maxOk);
    int value = settings.value("value").toInt(&valueOk);
    settings.endGroup();

    if (!minOk || !maxOk || min >= max) {
        min = fallback.min;
        max = fallback.max;
    }
    if (!valueOk || value < min || value > max) {
        value = std::clamp(fallback.value, min, max);
    }
    return {value, min, max};
}

void saveParam(QSettings& settings, const QString& group, const ParamSetting& param) {
    settings.beginGroup(group);
    settings.setValue("value", param.value);
    settings.setValue("min", param.min);
    settings.setValue("max", param.max);
    settings.endGroup();
}

int loadInt(QSettings& settings, const QString& key, int min, int max, int fallback) {
    bool ok = false;
    const int value = settings.value(key).toInt(&ok);
    return (ok && value >= min && value <= max) ? value : fallback;
}

double loadDouble(QSettings& settings, const QString& key, double min, double max, double fallback) {
    bool ok = false;
    const double value = settings.value(key).toDouble(&ok);
    return (ok && value >= min && value <= max) ? value : fallback;
}
}

AppConfig::AppConfig(const QString& filePath)
    : filePath_(filePath),
      params_(config_defaults::detectionParams()),
      processing_(config_defaults::kProcessing),
      render_(config_defaults::kRender),
      camera_(config_defaults::cameraParams()) {
    load();
}

DetectionParams& AppConfig::detection() {
    return params_;
}

const DetectionParams& AppConfig::detection() const {
    return params_;
}

const ProcessingParams& AppConfig::processing() const {
    return processing_;
}

const RenderParams& AppConfig::render() const {
    return render_;
}

const CameraParams& AppConfig::camera() const {
    return camera_;
}

void AppConfig::load() {
    if (!QFileInfo::exists(filePath_)) {
        return;
    }

    QSettings settings(filePath_, QSettings::IniFormat);
    params_.sensitivity = loadParam(settings, "sensitivity", config_defaults::kSensitivity);
    params_.edgeThreshold = loadParam(settings, "edge_threshold", config_defaults::kEdgeThreshold);
    params_.minDistancePercent = loadParam(settings, "min_distance_percent", config_defaults::kMinDistancePercent);
    params_.minRadiusPercent = loadParam(settings, "min_radius_percent", config_defaults::kMinRadiusPercent);
    params_.maxRadiusPercent = loadParam(settings, "max_radius_percent", config_defaults::kMaxRadiusPercent);

    if (params_.minRadiusPercent.value > params_.maxRadiusPercent.value) {
        params_.minRadiusPercent.value = std::clamp(
            config_defaults::kMinRadiusPercent.value,
            params_.minRadiusPercent.min,
            params_.minRadiusPercent.max);
        params_.maxRadiusPercent.value = std::clamp(
            config_defaults::kMaxRadiusPercent.value,
            params_.maxRadiusPercent.min,
            params_.maxRadiusPercent.max);
    }

    settings.beginGroup("processing");
    processing_.intervalMs = loadInt(settings, "interval_ms", 50, 10000, config_defaults::kProcessing.intervalMs);
    processing_.blurKernelSize = loadInt(settings, "blur_kernel_size", 3, 31, config_defaults::kProcessing.blurKernelSize);
    if (processing_.blurKernelSize % 2 == 0) {
        processing_.blurKernelSize = config_defaults::kProcessing.blurKernelSize;
    }
    processing_.dp = loadDouble(settings, "dp", 1.0, 3.0, config_defaults::kProcessing.dp);
    settings.endGroup();

    settings.beginGroup("render");
    render_.circleColorB = loadInt(settings, "circle_color_b", 0, 255, config_defaults::kRender.circleColorB);
    render_.circleColorG = loadInt(settings, "circle_color_g", 0, 255, config_defaults::kRender.circleColorG);
    render_.circleColorR = loadInt(settings, "circle_color_r", 0, 255, config_defaults::kRender.circleColorR);
    render_.circleThickness = loadInt(settings, "circle_thickness", 1, 20, config_defaults::kRender.circleThickness);
    settings.endGroup();

    settings.beginGroup("camera");
    camera_.deviceIndex = loadInt(settings, "device_index", 0, 16, config_defaults::kCameraDeviceIndex);
    const QString path = settings.value("fallback_video_path").toString();
    camera_.fallbackVideoPath = path.trimmed().isEmpty() ? config_defaults::cameraParams().fallbackVideoPath : path;
    settings.endGroup();
}

void AppConfig::save() {
    QSettings settings(filePath_, QSettings::IniFormat);
    saveParam(settings, "sensitivity", params_.sensitivity);
    saveParam(settings, "edge_threshold", params_.edgeThreshold);
    saveParam(settings, "min_distance_percent", params_.minDistancePercent);
    saveParam(settings, "min_radius_percent", params_.minRadiusPercent);
    saveParam(settings, "max_radius_percent", params_.maxRadiusPercent);

    settings.beginGroup("processing");
    settings.setValue("interval_ms", processing_.intervalMs);
    settings.setValue("blur_kernel_size", processing_.blurKernelSize);
    settings.setValue("dp", processing_.dp);
    settings.endGroup();

    settings.beginGroup("render");
    settings.setValue("circle_color_b", render_.circleColorB);
    settings.setValue("circle_color_g", render_.circleColorG);
    settings.setValue("circle_color_r", render_.circleColorR);
    settings.setValue("circle_thickness", render_.circleThickness);
    settings.endGroup();

    settings.beginGroup("camera");
    settings.setValue("device_index", camera_.deviceIndex);
    settings.setValue("fallback_video_path", camera_.fallbackVideoPath);
    settings.endGroup();

    settings.sync();
}
