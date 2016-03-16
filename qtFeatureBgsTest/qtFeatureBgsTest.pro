#-------------------------------------------------
#
# Project created by QtCreator 2016-03-15T18:24:20
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = qtFeatureBgsTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

cvroot=/mnt/hgfs/D/git
include($$cvroot/scripts/opencv_qt.pro)
include($$cvroot/scripts/bgs_qt.pro)
SOURCES += main.cpp
