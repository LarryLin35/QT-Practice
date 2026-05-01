#ifndef CAMERA_CAMERAWORKER_H
#define CAMERA_CAMERAWORKER_H

#include <atomic>
#include <thread>

#include <QObject>

#include "storage/framequeue.h"

class CameraWorker : public QObject {
    Q_OBJECT

public:
    explicit CameraWorker(FrameQueue* frameQueue, QObject* parent = nullptr);
    ~CameraWorker() override;

    void startCapture();
    void stopCapture();

signals:
    void cameraConnected();
    void cameraError(const QString& message);
    void cameraStopped();

private:
    void captureLoop();

    FrameQueue* frameQueue_;
    std::atomic<bool> running_;
    std::thread workerThread_;
};

#endif
