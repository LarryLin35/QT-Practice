#include "processor/circleprocessor.h"

#include <algorithm>

#include <opencv2/imgproc.hpp>

#include "config/configdefaults.h"

CircleProcessor::CircleProcessor(FrameQueue* frameQueue, QObject* parent)
    : QObject(parent),
      frameQueue_(frameQueue),
      running_(true),
      processingEnabled_(false),
      resetRequested_(false),
      sensitivity_(config_defaults::kSensitivity.value),
      edgeThreshold_(config_defaults::kEdgeThreshold.value),
      minDistancePercent_(config_defaults::kMinDistancePercent.value),
      minRadiusPercent_(config_defaults::kMinRadiusPercent.value),
      maxRadiusPercent_(config_defaults::kMaxRadiusPercent.value),
      intervalMs_(config_defaults::kProcessing.intervalMs),
      blurKernelSize_(config_defaults::kProcessing.blurKernelSize),
      dp_(config_defaults::kProcessing.dp),
      circleColorB_(config_defaults::kRender.circleColorB),
      circleColorG_(config_defaults::kRender.circleColorG),
      circleColorR_(config_defaults::kRender.circleColorR),
      circleThickness_(config_defaults::kRender.circleThickness),
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

void CircleProcessor::setProcessingParams(const ProcessingParams& params) {
    intervalMs_ = params.intervalMs;
    blurKernelSize_ = params.blurKernelSize;
    dp_ = params.dp;
}

void CircleProcessor::setRenderParams(const RenderParams& params) {
    circleColorB_ = params.circleColorB;
    circleColorG_ = params.circleColorG;
    circleColorR_ = params.circleColorR;
    circleThickness_ = params.circleThickness;
}

void CircleProcessor::processLoop() {
    auto lastProcessedAt = std::chrono::steady_clock::now() - std::chrono::milliseconds(intervalMs_.load());

    while (running_) {
        const std::chrono::milliseconds interval(intervalMs_.load());

        if (!processingEnabled_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        if (resetRequested_.exchange(false)) {
            lastProcessedAt = std::chrono::steady_clock::now() - interval;
        }

        if (!frameQueue_->waitForFrame(std::chrono::milliseconds(200))) {
            continue;
        }

        const auto now = std::chrono::steady_clock::now();
        if (now - lastProcessedAt < interval) {
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
    cv::medianBlur(gray, gray, blurKernelSize_.load());

    const int minRadius = gray.rows * minRadiusPercent_ / 100;
    const int maxRadius = std::max(minRadius, gray.rows * maxRadiusPercent_ / 100);

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(
        gray,
        circles,
        cv::HOUGH_GRADIENT,
        dp_.load(),
        std::max(1.0, gray.rows * minDistancePercent_ / 100.0),
        std::max(1, edgeThreshold_.load()),
        std::max(1, sensitivity_.load()),
        minRadius,
        maxRadius);

    const cv::Scalar circleColor(circleColorB_.load(), circleColorG_.load(), circleColorR_.load());
    const int circleThickness = circleThickness_.load();
    for (const cv::Vec3f& circle : circles) {
        const cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
        const int radius = cvRound(circle[2]);
        cv::circle(result, center, radius, circleColor, circleThickness);
    }

    return result;
}
