#include "camera/cameraworker.h"

#include <chrono>

#include <opencv2/videoio.hpp>

namespace {
const char* kFallbackVideoPath = "../test/test_video_01.mp4";
}

CameraWorker::CameraWorker(FrameQueue* frameQueue, QObject* parent)
    : QObject(parent), frameQueue_(frameQueue), running_(false), retryRequested_(false) {
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

    if (capture.open(0)) {
        emit cameraConnected("Camera Connected", false);
    } else if (capture.open(kFallbackVideoPath)) {
        usingFallbackVideo = true;
        emit cameraConnected("Fallback Video OK", true);
    } else {
        running_ = false;
        emit cameraError("No camera or fallback video source found.");
        return;
    }

    cv::Mat frame;
    while (running_) {
        if (usingFallbackVideo && retryRequested_.exchange(false)) {
            cv::VideoCapture cameraCapture;
            if (cameraCapture.open(0)) {
                capture = cameraCapture;
                usingFallbackVideo = false;
                emit cameraConnected("Camera Connected", false);
            } else {
                emit cameraRetryFailed();
            }
        }

        if (!capture.read(frame) || frame.empty()) {
            if (usingFallbackVideo) {
                capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            continue;
        }

        frameQueue_->push(frame);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    capture.release();
    emit cameraStopped();
}
