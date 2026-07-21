#include <QApplication>
#include <QDebug>

#include "config/appconfig.h"
#include "log/logger.h"
#include "ui/mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    Logger logger(QStringLiteral("../test"));
    qInfo() << "Application starting";

    AppConfig config(QStringLiteral("../test/config.ini"));

    MainWindow window(config);
    window.show();

    const int result = app.exec();
    config.save();
    return result;
}
