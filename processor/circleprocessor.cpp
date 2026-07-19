#include "processor/circleprocessor.h"

#include <algorithm>

#include <opencv2/imgproc.hpp>

CircleProcessor::CircleProcessor(FrameQueue* frameQueue, QObject* parent)
    : QObject(parent),
      frameQueue_(frameQueue),
      running_(true),
      processingEnabled_(false),
      resetRequested_(false),
      sensitivity_(35),
      edgeThreshold_(100),
      minDistancePercent_(33),
      minRadiusPercent_(10),
      maxRadiusPercent_(50),
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

void CircleProcessor::setSensitivity(int value) {
    sensitivity_ = value;
}

void CircleProcessor::setEdgeThreshold(int value) {
    edgeThreshold_ = value;
}

void CircleProcessor::setMinDistancePercent(int value) {
    minDistancePercent_ = value;
}

void CircleProcessor::setRadiusRangePercent(int minPercent, int maxPercent) {
    minRadiusPercent_ = minPercent;
    maxRadiusPercent_ = maxPercent;
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

    const int minRadius = gray.rows * minRadiusPercent_ / 100;
    const int maxRadius = std::max(minRadius, gray.rows * maxRadiusPercent_ / 100);

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(
        gray,
        circles,
        cv::HOUGH_GRADIENT,
        1.0,
        std::max(1.0, gray.rows * minDistancePercent_ / 100.0),
        std::max(1, edgeThreshold_.load()),
        std::max(1, sensitivity_.load()),
        minRadius,
        maxRadius);

    for (const cv::Vec3f& circle : circles) {
        const cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
        const int radius = cvRound(circle[2]);
        cv::circle(result, center, radius, cv::Scalar(0, 0, 255), 3);
    }

    return result;
}
