#ifndef STORAGE_FRAMEQUEUE_H
#define STORAGE_FRAMEQUEUE_H

#include <chrono>
#include <condition_variable>
#include <mutex>

#include <opencv2/opencv.hpp>

class FrameQueue {
public:
    FrameQueue();

    void push(const cv::Mat& frame);
    bool waitForFrame(std::chrono::milliseconds timeout);
    cv::Mat takeLatest();
    void stop();
    void reset();

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    cv::Mat latestFrame_;
    bool hasFrame_;
    bool stopped_;
};

#endif
