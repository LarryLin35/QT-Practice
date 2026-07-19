#ifndef CAMERA_CAMERAWORKER_H
#define CAMERA_CAMERAWORKER_H

#include <atomic>
#include <string>
#include <thread>

#include <QObject>
#include <QString>

#include "storage/framequeue.h"

class CameraWorker : public QObject {
    Q_OBJECT

public:
    CameraWorker(FrameQueue* frameQueue, int deviceIndex, const QString& fallbackVideoPath,
                 QObject* parent = nullptr);
    ~CameraWorker() override;

    void startCapture();
    void stopCapture();
    void requestCameraRetry();

signals:
    void cameraConnected(const QString& statusMessage, bool usingFallbackVideo);
    void cameraError(const QString& message);
    void cameraRetryFailed();
    void cameraStopped();

private:
    void captureLoop();

    FrameQueue* frameQueue_;
    int deviceIndex_;
    std::string fallbackVideoPath_;
    std::atomic<bool> running_;
    std::atomic<bool> retryRequested_;
    std::thread workerThread_;
};

#endif
