QT += core widgets sql charts
QT += charts

CONFIG += c++17

# 编译器优化选项，减少内存使用
QMAKE_CXXFLAGS += -O1
QMAKE_CXXFLAGS += -fno-inline-functions
QMAKE_CXXFLAGS += -fno-inline-small-functions

# 增加编译器内存限制
QMAKE_CXXFLAGS += -Wl,--stack,16777216

# Disable filesystem to avoid compatibility issues
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
DEFINES += QT_NO_FILESYSTEM

TARGET = RailwayAnalysis
TEMPLATE = app

# 源文件
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/datamanager.cpp \
    src/station.cpp \
    src/train.cpp \
    src/passengerflow.cpp \
    src/analysisengine.cpp \
    src/tablewidget.cpp \
    src/predictionmodel.cpp \
    src/qcustomplot.cpp \
    src/chartwidget.cpp

# 头文件
HEADERS += \
    include/mainwindow.h \
    include/datamanager.h \
    include/station.h \
    include/train.h \
    include/passengerflow.h \
    include/analysisengine.h \
    include/tablewidget.h \
    include/predictionmodel.h \
    include/qcustomplot.h \
    include/chartwidget.h

# 包含路径
INCLUDEPATH += include

# 数据文件
DISTFILES += \
    客运站点（站点名称、站点编号、备注）.csv \
    列车表（列车编码、列车代码、列车运量）.csv \
    运营线路客运站（运营线路编码、站点id、线路站点id、上一站id、运营线路站间距离 、下一站id、运输距离、线路代码）.csv \
    高铁客运量（成都--重庆）（运营线路编码、列车编码、站点id、日期、到达时间、出发时间、上客量、下客量等，起点站、终点站、票价、收入等）(3).csv

# 编译设置
DEFINES += QT_DEPRECATED_WARNINGS

# 调试信息
CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}

# 发布设置
CONFIG(release, debug|release) {
    DEFINES += NDEBUG
} 
