#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    
    // 保持 OpenCV 測試（可選）
    std::cout << "OpenCV Version: " << CV_VERSION << std::endl;

    QWidget window;
    window.setWindowTitle("Simple Toggle UI");
    window.resize(300, 200);

    QVBoxLayout *layout = new QVBoxLayout(&window);

    QLabel *label = new QLabel("Hello World!");
    label->setAlignment(Qt::AlignCenter);
    label->hide(); // 初始狀態設為隱藏

    QPushButton *button = new QPushButton("Show / Hide");

    layout->addWidget(label);
    layout->addWidget(button);

    // 點擊事件：切換 label 的顯示/隱藏
    QObject::connect(button, &QPushButton::clicked, [label]() {
        label->setVisible(!label->isVisible());
    });

    window.show();

    return a.exec();
}
