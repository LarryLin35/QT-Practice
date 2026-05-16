#include "ui/mainwindow.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMetaObject>
#include <QPixmap>
#include <QPushButton>
#include <QTextCursor>
#include <QTextEdit>
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
      captureLight_(nullptr),
      processingLight_(nullptr),
      imageLabel_(nullptr),
      cameraButton_(nullptr),
      processingButton_(nullptr),
      messageBoard_(nullptr),
      uiState_(UiState::Idle),
      captureStatusText_("Connect Camera"),
      cameraWorker_(new CameraWorker(&frameQueue_)),
      processorWorker_(new CircleProcessor(&frameQueue_)) {
    qRegisterMetaType<cv::Mat>("cv::Mat");
    setupUi();

    connect(cameraWorker_, &CameraWorker::cameraConnected, this, &MainWindow::onCameraConnected);
    connect(cameraWorker_, &CameraWorker::cameraError, this, &MainWindow::onCameraError);
    connect(cameraWorker_, &CameraWorker::cameraStopped, this, &MainWindow::onCameraStopped);
    connect(processorWorker_, &CircleProcessor::frameProcessed, this, &MainWindow::onFrameProcessed);
}

MainWindow::~MainWindow() {
    stopWorkers();
}

void MainWindow::handleCameraButton() {
    if (uiState_ == UiState::Idle) {
        updateUiState(UiState::Connecting, {});
        appendMessage("Searching for camera...");
        cameraWorker_->startCapture();
    }
}

void MainWindow::handleProcessingButton() {
    if (uiState_ == UiState::ReadyToProcess) {
        processorWorker_->setProcessingEnabled(true);
        updateUiState(UiState::Processing, {});
        appendMessage("Circle detection is running once every second.");
        return;
    }

    if (uiState_ == UiState::Processing) {
        processorWorker_->setProcessingEnabled(false);
        updateUiState(UiState::ReadyToProcess, {});
        appendMessage("Circle detection stopped.");
    }
}

void MainWindow::onCameraConnected(const QString& statusMessage) {
    updateUiState(UiState::ReadyToProcess, statusMessage);
    appendMessage(statusMessage);
}

void MainWindow::onCameraError(const QString& message) {
    updateUiState(UiState::Idle, message);
    appendMessage(message);
}

void MainWindow::onCameraStopped() {
    if (uiState_ == UiState::Connecting) {
        updateUiState(UiState::Idle, "Camera connection stopped.");
        appendMessage("Camera connection stopped.");
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
    auto* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    imageLabel_ = new QLabel(this);
    imageLabel_->setMinimumSize(480, 360);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setStyleSheet("background-color: #202020; color: white;");
    imageLabel_->setText("Processed image will appear here.");

    auto* controlPanel = new QWidget(this);
    controlPanel->setMinimumWidth(260);

    auto* controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(12);

    auto* cameraRow = new QHBoxLayout();
    cameraRow->setSpacing(8);
    captureLight_ = new QLabel(this);
    captureLight_->setFixedSize(14, 14);
    cameraButton_ = new QPushButton("Connect Camera", this);
    cameraButton_->setMinimumHeight(36);
    cameraRow->addWidget(captureLight_);
    cameraRow->addWidget(cameraButton_);

    auto* processingRow = new QHBoxLayout();
    processingRow->setSpacing(8);
    processingLight_ = new QLabel(this);
    processingLight_->setFixedSize(14, 14);
    processingButton_ = new QPushButton("Start Processing", this);
    processingButton_->setMinimumHeight(36);
    processingRow->addWidget(processingLight_);
    processingRow->addWidget(processingButton_);

    messageBoard_ = new QTextEdit(this);
    messageBoard_->setReadOnly(true);
    messageBoard_->setMinimumHeight(180);
    messageBoard_->setStyleSheet(
        "QTextEdit {"
        "background-color: #f7f7f4;"
        "border: 1px solid #b8b8b0;"
        "color: #202020;"
        "font-family: Consolas, monospace;"
        "font-size: 12px;"
        "}");

    connect(cameraButton_, &QPushButton::clicked, this, &MainWindow::handleCameraButton);
    connect(processingButton_, &QPushButton::clicked, this, &MainWindow::handleProcessingButton);

    controlLayout->addLayout(cameraRow);
    controlLayout->addLayout(processingRow);
    controlLayout->addWidget(messageBoard_, 1);

    mainLayout->addWidget(imageLabel_, 1);
    mainLayout->addWidget(controlPanel, 1);

    setCentralWidget(centralWidget);
    setWindowTitle("Qt Camera Circle Detection");
    resize(960, 600);
    updateUiState(UiState::Idle, {});
    appendMessage("Ready. Connect camera to start.");
}

void MainWindow::appendMessage(const QString& message) {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    messageBoard_->append(QString("[%1] %2").arg(timestamp, message));
    messageBoard_->moveCursor(QTextCursor::End);
}

void MainWindow::updateIndicator(QLabel* light, const QString& color) {
    light->setStyleSheet(QString(
        "background-color: %1;"
        "border-radius: 7px;"
        "border: 1px solid rgba(255, 255, 255, 80);")
                                     .arg(color));
}

void MainWindow::updateUiState(UiState state, const QString& statusMessage) {
    uiState_ = state;
    const QString inactiveColor = "#d64545";
    const QString activeColor = "#2eb85c";

    switch (uiState_) {
    case UiState::Idle:
        captureStatusText_ = statusMessage.isEmpty() ? "Connect Camera" : statusMessage;
        updateIndicator(captureLight_, inactiveColor);
        updateIndicator(processingLight_, inactiveColor);
        cameraButton_->setEnabled(true);
        cameraButton_->setText(captureStatusText_);
        processingButton_->setEnabled(false);
        processingButton_->setText("Start Processing");
        break;
    case UiState::Connecting:
        updateIndicator(captureLight_, inactiveColor);
        updateIndicator(processingLight_, inactiveColor);
        cameraButton_->setEnabled(false);
        cameraButton_->setText("Connecting...");
        processingButton_->setEnabled(false);
        processingButton_->setText("Start Processing");
        break;
    case UiState::ReadyToProcess:
        if (!statusMessage.isEmpty()) {
            captureStatusText_ = statusMessage;
        }
        updateIndicator(captureLight_, activeColor);
        updateIndicator(processingLight_, inactiveColor);
        cameraButton_->setEnabled(false);
        cameraButton_->setText(captureStatusText_);
        processingButton_->setEnabled(true);
        processingButton_->setText("Start Processing");
        break;
    case UiState::Processing:
        updateIndicator(captureLight_, activeColor);
        updateIndicator(processingLight_, activeColor);
        cameraButton_->setEnabled(false);
        cameraButton_->setText(captureStatusText_);
        processingButton_->setEnabled(true);
        processingButton_->setText("Stop Processing");
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
