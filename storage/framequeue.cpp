#include "storage/framequeue.h"

FrameQueue::FrameQueue() : hasFrame_(false), stopped_(false) {
}

void FrameQueue::push(const cv::Mat& frame) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stopped_) {
        return;
    }

    latestFrame_ = frame.clone();
    hasFrame_ = true;
    condition_.notify_one();
}

bool FrameQueue::waitForFrame(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return condition_.wait_for(lock, timeout, [this]() { return hasFrame_ || stopped_; });
}

cv::Mat FrameQueue::takeLatest() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!hasFrame_) {
        return {};
    }

    hasFrame_ = false;
    return latestFrame_.clone();
}

void FrameQueue::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    stopped_ = true;
    condition_.notify_all();
}

void FrameQueue::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    latestFrame_.release();
    hasFrame_ = false;
    stopped_ = false;
}
