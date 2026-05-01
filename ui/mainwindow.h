#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QMainWindow>

#include "camera/cameraworker.h"
#include "processor/circleprocessor.h"
#include "storage/framequeue.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void handlePrimaryButton();
    void onCameraConnected();
    void onCameraError(const QString& message);
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
    void updateUiState(UiState state, const QString& statusMessage);
    void stopWorkers();

    QLabel* statusLabel_;
    QLabel* imageLabel_;
    QPushButton* primaryButton_;

    UiState uiState_;
    FrameQueue frameQueue_;
    CameraWorker* cameraWorker_;
    CircleProcessor* processorWorker_;
};

#endif
