# This file is generated automatically. Do not edit.
# Use project properties -> Build -> Qt -> Expert -> Custom Definitions.
TEMPLATE = app
DESTDIR = dist/Release/MinGW-Windows
TARGET = cockroach
VERSION = 1.0.0
CONFIG -= debug_and_release app_bundle lib_bundle
CONFIG += release 
PKGCONFIG +=
QT = core gui svg
SOURCES += MainForm.cpp PlotForm.cpp main.cpp
HEADERS += MainForm.h PlotForm.h ftd2xx.h structs.h
FORMS += MainForm.ui PlotForm.ui
RESOURCES +=
TRANSLATIONS +=
OBJECTS_DIR = build/Release/MinGW-Windows
MOC_DIR = 
RCC_DIR = 
UI_DIR = 
QMAKE_CC = gcc
QMAKE_CXX = g++
DEFINES += 
INCLUDEPATH += ../../../usr/qt5/5.1.0/mingw48_32/include/qwt ../../../usr/qt5/5.1.0/mingw48_32/include/qnavwidget 
LIBS += 
LIBS += -lQNavWidget -lqwt
QT += widgets
