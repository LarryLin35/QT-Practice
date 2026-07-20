#ifndef CONFIG_APPCONFIG_H
#define CONFIG_APPCONFIG_H

#include <QString>

#include "config/detectionparams.h"

class AppConfig {
public:
    explicit AppConfig(const QString& filePath);

    DetectionParams& detection();
    const DetectionParams& detection() const;
    const ProcessingParams& processing() const;
    const RenderParams& render() const;
    const CameraParams& camera() const;

    void save();

private:
    void load();

    QString filePath_;
    DetectionParams params_;
    ProcessingParams processing_;
    RenderParams render_;
    CameraParams camera_;
};

#endif
