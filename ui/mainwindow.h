#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include "camera/cameraworker.h"
#include "processor/circleprocessor.h"
#include "storage/framequeue.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QTextEdit;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void handleCameraButton();
    void handleProcessingButton();
    void onCameraConnected(const QString& statusMessage, bool usingFallbackVideo);
    void onCameraError(const QString& message);
    void onCameraRetryFailed();
    void onCameraStopped();
    void onFrameProcessed(const cv::Mat& frame);

private:
    enum class UiState {
        Idle,
        Connecting,
        ReadyToProcess,
        Processing
    };

    void setupUi();
    void appendMessage(const QString& message);
    void updateIndicator(QLabel* light, const QString& color);
    void updateUiState(UiState state, const QString& statusMessage);
    void stopWorkers();

    QLabel* captureLight_;
    QLabel* processingLight_;
    QLabel* imageLabel_;
    QPushButton* cameraButton_;
    QPushButton* processingButton_;
    QTextEdit* messageBoard_;

    UiState uiState_;
    QString captureStatusText_;
    bool usingFallbackVideo_;
    FrameQueue frameQueue_;
    CameraWorker* cameraWorker_;
    CircleProcessor* processorWorker_;
};

#endif
