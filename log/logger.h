#ifndef LOG_LOGGER_H
#define LOG_LOGGER_H

#include <QString>

// Configures file-based logging for the whole application.
//
// On construction it opens a size-rotating log file inside `logDir` and routes
// every Qt logging call (qDebug/qInfo/qWarning/qCritical/qFatal and their qC*
// category variants) into it. Business code keeps using the normal Qt logging
// macros -- only main() needs to create a single Logger instance.
//
// Rotation keeps the on-disk footprint bounded at 3 files of 5 MiB each
// (15 MiB total):
//   app.log    <- current
//   app.1.log  <- previous
//   app.2.log  <- oldest (overwritten on the next rotation)
class Logger {
public:
    explicit Logger(const QString& logDir = QStringLiteral("../test"));
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif
