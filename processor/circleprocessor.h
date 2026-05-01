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

signals:
    void frameProcessed(const cv::Mat& frame);

private:
    void processLoop();
    cv::Mat detectCircles(const cv::Mat& frame) const;

    FrameQueue* frameQueue_;
    std::atomic<bool> running_;
    std::atomic<bool> processingEnabled_;
    std::atomic<bool> resetRequested_;
    std::thread workerThread_;
};

#endif
