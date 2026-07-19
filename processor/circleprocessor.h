#ifndef PROCESSOR_CIRCLEPROCESSOR_H
#define PROCESSOR_CIRCLEPROCESSOR_H

#include <atomic>
#include <chrono>
#include <thread>

#include <QObject>

#include "storage/framequeue.h"

Q_DECLARE_METATYPE(cv::Mat)

class CircleProcessor : public QObject {
    Q_OBJECT

public:
    explicit CircleProcessor(FrameQueue* frameQueue, QObject* parent = nullptr);
    ~CircleProcessor() override;

    void stop();
    void setProcessingEnabled(bool enabled);

    void setSensitivity(int value);
    void setEdgeThreshold(int value);
    void setMinDistancePercent(int value);
    void setRadiusRangePercent(int minPercent, int maxPercent);

signals:
    void frameProcessed(const cv::Mat& frame);

private:
    void processLoop();
    cv::Mat detectCircles(const cv::Mat& frame) const;

    FrameQueue* frameQueue_;
    std::atomic<bool> running_;
    std::atomic<bool> processingEnabled_;
    std::atomic<bool> resetRequested_;
    std::atomic<int> sensitivity_;
    std::atomic<int> edgeThreshold_;
    std::atomic<int> minDistancePercent_;
    std::atomic<int> minRadiusPercent_;
    std::atomic<int> maxRadiusPercent_;
    std::thread workerThread_;
};

#endif
