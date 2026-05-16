#include "camera/cameraworker.h"

#include <chrono>

#include <opencv2/videoio.hpp>

namespace {
const char* kFallbackVideoPath = "../test/test_video_01.mp4";
}

CameraWorker::CameraWorker(FrameQueue* frameQueue, QObject* parent)
    : QObject(parent), frameQueue_(frameQueue), running_(false) {
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
    running_ = true;
    workerThread_ = std::thread(&CameraWorker::captureLoop, this);
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
        emit cameraConnected("Camera Connected");
    } else if (capture.open(kFallbackVideoPath)) {
        usingFallbackVideo = true;
        emit cameraConnected("Fallback Video OK");
    } else {
        running_ = false;
        emit cameraError("No camera or fallback video source found.");
        return;
    }

    cv::Mat frame;
    while (running_) {
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
