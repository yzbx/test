#-------------------------------------------------
#
# Project created by QtCreator 2016-03-15T18:24:20
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = qtFeatureBgsTest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app

cvroot=/home/yzbx/git/yzbxLib
include($$cvroot/scripts/opencv_qt_e420.pro)
include($$cvroot/scripts/bgs_qt_e420.pro)

message ($$bgsroot/debug/libbgs.so)
LIBS +=$$bgsroot/debug/libbgs.so

INCLUDEPATH +=$$bgsroot/package_bgs

SOURCES += main.cpp \
    yzbx_tracking.cpp

HEADERS += \
    yzbx_tracking.h \
AdaptiveBackgroundLearning.h
