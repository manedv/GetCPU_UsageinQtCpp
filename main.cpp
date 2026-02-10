#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QThread>
#include <QtGlobal>

#ifdef Q_OS_WIN
#include <windows.h>
#include <pdh.h>
#pragma comment(lib, "pdh.lib")

// Use PDH (same as Task Manager) for accurate CPU %
static double getCpuUsagePercent() {
    static HQUERY query = nullptr;
    static HCOUNTER counter = nullptr;
    static bool initialized = false;

    if (!initialized) {
        if (PdhOpenQueryW(nullptr, 0, &query) != ERROR_SUCCESS)
            return -1.0;
        // Same counter as Task Manager: total processor time
        if (PdhAddEnglishCounterW(query, L"\\Processor(_Total)\\% Processor Time", 0, &counter) != ERROR_SUCCESS) {
            PdhCloseQuery(query);
            return -1.0;
        }
        PdhCollectQueryData(query); // first sample (required before value is valid)
        initialized = true;
    }

    QThread::msleep(100); // sample interval (closer to Task Manager's refresh)
    if (PdhCollectQueryData(query) != ERROR_SUCCESS)
        return -1.0;

    PDH_FMT_COUNTERVALUE fmt = {};
    if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &fmt) != ERROR_SUCCESS)
        return -1.0;
    // 0 = valid, 1 = new data (success); other values = error
    if (fmt.CStatus != 0 && fmt.CStatus != 1)
        return -1.0;

    return fmt.doubleValue;
}
#else
static double getCpuUsagePercent() {
    Q_UNUSED(0);
    return -1.0; // Implement for Linux/macOS if needed
}
#endif

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    qInfo() << "Qt CPU usage monitor (every 30 sec). Press Ctrl+C to exit.\n";

    auto printCpuUsage = []() {
        double usage = getCpuUsagePercent();
        if (usage < 0)
            qWarning() << "Failed to get CPU usage.";
        else
            qInfo() << "CPU usage:" << usage << "%";
    };

    QTimer *timer = new QTimer(&app);
    QObject::connect(timer, &QTimer::timeout, printCpuUsage);
    timer->start(2000); // every 2 seconds
    QTimer::singleShot(0, &app, printCpuUsage); // print once at startup

    return app.exec();
}
