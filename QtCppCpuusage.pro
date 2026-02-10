QT       -= gui
QT       += core
CONFIG   += c++11 console
CONFIG   -= app_bundle
TARGET   = QtCppCpuusage
TEMPLATE = app
SOURCES  += main.cpp

# Windows: PDH for Task Manager–accurate CPU usage
win32 {
    LIBS += -lpdh
}
