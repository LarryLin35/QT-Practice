#include "ui/mainwindow.h"

#include <QImage>
#include <QLabel>
#include <QMetaObject>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include <opencv2/imgproc.hpp>

namespace {
QImage matToQImage(const cv::Mat& frame) {
    if (frame.empty()) {
        return {};
    }

    cv::Mat rgbFrame;
    cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
    return QImage(
               rgbFrame.data,
               rgbFrame.cols,
               rgbFrame.rows,
               static_cast<int>(rgbFrame.step),
               QImage::Format_RGB888)
        .copy();
}
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      statusLabel_(nullptr),
      imageLabel_(nullptr),
      primaryButton_(nullptr),
      uiState_(UiState::Idle),
      cameraWorker_(new CameraWorker(&frameQueue_)),
      processorWorker_(new CircleProcessor(&frameQueue_)) {
    qRegisterMetaType<cv::Mat>("cv::Mat");
    setupUi();

    connect(cameraWorker_, &CameraWorker::cameraConnected, this, &MainWindow::onCameraConnected);
    connect(cameraWorker_, &CameraWorker::cameraError, this, &MainWindow::onCameraError);
    connect(cameraWorker_, &CameraWorker::cameraStopped, this, &MainWindow::onCameraStopped);
    connect(processorWorker_, &CircleProcessor::frameProcessed, this, &MainWindow::onFrameProcessed);
    updateUiState(UiState::Idle, "Press the button to connect the camera.");
}

MainWindow::~MainWindow() {
    stopWorkers();
}

void MainWindow::handlePrimaryButton() {
    if (uiState_ == UiState::Idle) {
        updateUiState(UiState::Connecting, "Searching for camera...");
        cameraWorker_->startCapture();
        return;
    }

    if (uiState_ == UiState::ReadyToProcess) {
        processorWorker_->setProcessingEnabled(true);
        updateUiState(UiState::Processing, "Circle detection is running once every second.");
        return;
    }

    if (uiState_ == UiState::Processing) {
        processorWorker_->setProcessingEnabled(false);
        updateUiState(UiState::ReadyToProcess, "Processing stopped. Press the button to run again.");
    }
}

void MainWindow::onCameraConnected() {
    updateUiState(UiState::ReadyToProcess, "Camera connected. Ready to start image processing.");
}

void MainWindow::onCameraError(const QString& message) {
    updateUiState(UiState::Idle, message);
}

void MainWindow::onCameraStopped() {
    if (uiState_ == UiState::Connecting) {
        updateUiState(UiState::Idle, "Camera connection stopped.");
    }
}

void MainWindow::onFrameProcessed(const cv::Mat& frame) {
    const QImage image = matToQImage(frame);
    if (image.isNull()) {
        return;
    }

    imageLabel_->setPixmap(QPixmap::fromImage(image).scaled(
        imageLabel_->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation));
}

void MainWindow::setupUi() {
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    statusLabel_ = new QLabel(this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setWordWrap(true);

    imageLabel_ = new QLabel(this);
    imageLabel_->setMinimumSize(640, 480);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setStyleSheet("background-color: #202020; color: white;");
    imageLabel_->setText("Processed image will appear here.");

    primaryButton_ = new QPushButton(this);
    connect(primaryButton_, &QPushButton::clicked, this, &MainWindow::handlePrimaryButton);

    layout->addWidget(statusLabel_);
    layout->addWidget(imageLabel_);
    layout->addWidget(primaryButton_);

    setCentralWidget(centralWidget);
    setWindowTitle("Qt Camera Circle Detection");
    resize(800, 700);
}

void MainWindow::updateUiState(UiState state, const QString& statusMessage) {
    uiState_ = state;
    statusLabel_->setText(statusMessage);

    switch (uiState_) {
    case UiState::Idle:
        primaryButton_->setEnabled(true);
        primaryButton_->setText("Connect Camera");
        break;
    case UiState::Connecting:
        primaryButton_->setEnabled(false);
        primaryButton_->setText("Connecting...");
        break;
    case UiState::ReadyToProcess:
        primaryButton_->setEnabled(true);
        primaryButton_->setText("Start Processing");
        break;
    case UiState::Processing:
        primaryButton_->setEnabled(true);
        primaryButton_->setText("Stop Processing");
        break;
    }
}

void MainWindow::stopWorkers() {
    if (!cameraWorker_ || !processorWorker_) {
        return;
    }

    processorWorker_->setProcessingEnabled(false);
    cameraWorker_->stopCapture();
    processorWorker_->stop();

    frameQueue_.stop();

    delete cameraWorker_;
    delete processorWorker_;
    cameraWorker_ = nullptr;
    processorWorker_ = nullptr;
}
