#include "camera/cameraworker.h"

#include <chrono>

#include <QLoggingCategory>

#include <opencv2/videoio.hpp>

namespace {
Q_LOGGING_CATEGORY(logCamera, "app.camera")
}

CameraWorker::CameraWorker(FrameQueue* frameQueue, int deviceIndex, const QString& fallbackVideoPath,
                           QObject* parent)
    : QObject(parent),
      frameQueue_(frameQueue),
      deviceIndex_(deviceIndex),
      fallbackVideoPath_(fallbackVideoPath.toStdString()),
      running_(false),
      retryRequested_(false) {
}

CameraWorker::~CameraWorker() {
    stopCapture();
}

void CameraWorker::startCapture() {
    if (running_) {
        return;
    }

    if (workerThread_.joinable()) {
        workerThread_.join();
    }

    frameQueue_->reset();
    retryRequested_ = false;
    running_ = true;
    workerThread_ = std::thread(&CameraWorker::captureLoop, this);
}

void CameraWorker::requestCameraRetry() {
    if (running_) {
        retryRequested_ = true;
    }
}

void CameraWorker::stopCapture() {
    running_ = false;
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void CameraWorker::captureLoop() {
    cv::VideoCapture capture;
    bool usingFallbackVideo = false;

    if (capture.open(deviceIndex_)) {
        qCInfo(logCamera) << "Camera opened on device index" << deviceIndex_;
        emit cameraConnected("Camera Connected", false);
    } else if (capture.open(fallbackVideoPath_)) {
        usingFallbackVideo = true;
        qCWarning(logCamera) << "Camera device" << deviceIndex_
                             << "unavailable; using fallback video:"
                             << QString::fromStdString(fallbackVideoPath_);
        emit cameraConnected("Fallback Video OK", true);
    } else {
        running_ = false;
        qCCritical(logCamera) << "No source found (device index" << deviceIndex_
                              << ", fallback:" << QString::fromStdString(fallbackVideoPath_) << ")";
        emit cameraError("No camera or fallback video source found.");
        return;
    }

    cv::Mat frame;
    bool readFailing = false;
    while (running_) {
        if (usingFallbackVideo && retryRequested_.exchange(false)) {
            cv::VideoCapture cameraCapture;
            if (cameraCapture.open(deviceIndex_)) {
                capture = cameraCapture;
                usingFallbackVideo = false;
                qCInfo(logCamera) << "Retry succeeded; switched to camera device" << deviceIndex_;
                emit cameraConnected("Camera Connected", false);
            } else {
                qCWarning(logCamera) << "Camera retry failed; staying on fallback video";
                emit cameraRetryFailed();
            }
        }

        if (!capture.read(frame) || frame.empty()) {
            if (usingFallbackVideo) {
                qCDebug(logCamera) << "Fallback video reached end; looping";
                capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            } else if (!readFailing) {
                readFailing = true;
                qCWarning(logCamera) << "Camera frame read failed; will keep retrying";
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            continue;
        }

        if (readFailing) {
            readFailing = false;
            qCInfo(logCamera) << "Camera frame read recovered";
        }

        frameQueue_->push(frame);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    capture.release();
    qCInfo(logCamera) << "Capture loop stopped";
    emit cameraStopped();
}
