#include "camera/cameraworker.h"

#include <opencv2/videoio.hpp>

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
    const std::string testStreamUrl = "../test/test_video_01.mp4";
    cv::VideoCapture capture(testStreamUrl);
    if (!capture.isOpened()) {
        capture.open(0);
    }

    if (!capture.isOpened()) {
        running_ = false;
        emit cameraError("Unable to connect to test stream or camera.");
        return;
    }

    emit cameraConnected();

    cv::Mat frame;
    while (running_) {
        if (!capture.read(frame) || frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            continue;
        }

        frameQueue_->push(frame);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    capture.release();
    emit cameraStopped();
}
