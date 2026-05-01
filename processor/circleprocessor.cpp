#include "processor/circleprocessor.h"

#include <opencv2/imgproc.hpp>

CircleProcessor::CircleProcessor(FrameQueue* frameQueue, QObject* parent)
    : QObject(parent),
      frameQueue_(frameQueue),
      running_(true),
      processingEnabled_(false),
      resetRequested_(false),
      workerThread_(&CircleProcessor::processLoop, this) {
}

CircleProcessor::~CircleProcessor() {
    stop();
}

void CircleProcessor::stop() {
    running_ = false;
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void CircleProcessor::setProcessingEnabled(bool enabled) {
    processingEnabled_ = enabled;
    if (enabled) {
        resetRequested_ = true;
    }
}

void CircleProcessor::processLoop() {
    auto lastProcessedAt = std::chrono::steady_clock::now() - std::chrono::seconds(1);

    while (running_) {
        if (!processingEnabled_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        if (resetRequested_.exchange(false)) {
            lastProcessedAt = std::chrono::steady_clock::now() - std::chrono::seconds(1);
        }

        if (!frameQueue_->waitForFrame(std::chrono::milliseconds(200))) {
            continue;
        }

        const auto now = std::chrono::steady_clock::now();
        if (now - lastProcessedAt < std::chrono::seconds(1)) {
            continue;
        }

        cv::Mat frame = frameQueue_->takeLatest();
        if (frame.empty()) {
            continue;
        }

        lastProcessedAt = now;
        emit frameProcessed(detectCircles(frame));
    }
}

cv::Mat CircleProcessor::detectCircles(const cv::Mat& frame) const {
    cv::Mat result = frame.clone();
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::medianBlur(gray, gray, 5);

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(
        gray,
        circles,
        cv::HOUGH_GRADIENT,
        1.0,
        gray.rows / 8.0,
        100.0,
        30.0,
        10,
        0);

    for (const cv::Vec3f& circle : circles) {
        const cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
        const int radius = cvRound(circle[2]);
        cv::circle(result, center, radius, cv::Scalar(0, 0, 255), 3);
    }

    return result;
}
