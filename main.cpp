#include <QApplication>

#include "config/appconfig.h"
#include "ui/mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    AppConfig config(QStringLiteral("../test/config.ini"));

    MainWindow window(config);
    window.show();

    const int result = app.exec();
    config.save();
    return result;
}
