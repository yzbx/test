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
LBPRoot=lib/LBP1
include($$cvroot/scripts/opencv_qt_e420.pro)
include($$cvroot/scripts/bgs_qt_e420.pro)

LIBS +=/usr/lib/x86_64-linux-gnu/libfftw3.so

INCLUDEPATH += $$LBPRoot \
lib/lbp

SOURCES += main.cpp \
#    yzbx_tracking.cpp \
#    yzbx_tracking_short.cpp \
    yzbx_utility.cpp \
    lbp_bgs.cpp \
    $$LBPRoot/LBP.cpp \
    npe_bgs.cpp \
    main_bgs.cpp \
    shadowremove.cpp \
    lib/lbp/lbp.cpp \
    lib/lbp/histogram.cpp \
    yzbx_frameinput.cpp \
    yzbx_cdnetbenchmark.cpp

HEADERS += \
#    yzbx_tracking.h \
#    yzbx_tracking_short.h \
    yzbx_utility.h \
    lbp_bgs.h \
    npe_bgs.h \
    main_bgs.h \
    bgslibrary.h \
    shadowremove.h \
    lib/lbp/lbp.hpp \
    lib/lbp/histogram.hpp \
    yzbx_config.h \
    yzbx_frameinput.h \
    yzbx_cdnetbenchmark.h
