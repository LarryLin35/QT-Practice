#include "log/logger.h"

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>

#include <QDir>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

namespace {

// 5 MiB per file, keeping 2 rotated files besides the active one. spdlog counts
// max_files as the number of *rotated* files, so 2 yields 3 files on disk
// (app.log + app.1.log + app.2.log) = 15 MiB total.
constexpr std::size_t kMaxFileBytes = 5 * 1024 * 1024;
constexpr std::size_t kMaxRotatedFiles = 2;

constexpr char kLoggerName[] = "app";

std::shared_ptr<spdlog::logger> g_fileLogger;
QtMessageHandler g_previousHandler = nullptr;

void forwardToSpdlog(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    if (!g_fileLogger) {
        return;
    }

    const char* category = (context.category != nullptr) ? context.category : "default";
    const std::string line =
        QStringLiteral("[%1] %2").arg(QString::fromUtf8(category), message).toStdString();

    switch (type) {
        case QtDebugMsg:    g_fileLogger->debug(line);    break;
        case QtInfoMsg:     g_fileLogger->info(line);     break;
        case QtWarningMsg:  g_fileLogger->warn(line);     break;
        case QtCriticalMsg: g_fileLogger->error(line);    break;
        case QtFatalMsg:    g_fileLogger->critical(line); break;
    }
}

}  // namespace

Logger::Logger(const QString& logDir) {
    QDir().mkpath(logDir);
    const QString filePath = QDir(logDir).filePath(QStringLiteral("app.log"));

    g_fileLogger = spdlog::rotating_logger_mt(
        kLoggerName, filePath.toStdString(), kMaxFileBytes, kMaxRotatedFiles);
    g_fileLogger->set_level(spdlog::level::debug);
    g_fileLogger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] [t:%t] %v");

    // Warnings and worse hit the disk immediately; lower levels are flushed
    // periodically so a crash still leaves recent context without paying a
    // disk write per debug line.
    g_fileLogger->flush_on(spdlog::level::warn);
    spdlog::flush_every(std::chrono::seconds(3));

    g_previousHandler = qInstallMessageHandler(forwardToSpdlog);
}

Logger::~Logger() {
    qInstallMessageHandler(g_previousHandler);
    g_previousHandler = nullptr;

    if (g_fileLogger) {
        g_fileLogger->flush();
        g_fileLogger.reset();
    }
    spdlog::drop(kLoggerName);
}
