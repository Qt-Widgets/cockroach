# This file is generated automatically. Do not edit.
# Use project properties -> Build -> Qt -> Expert -> Custom Definitions.
TEMPLATE = app
DESTDIR = dist/Debug/MinGW-Windows
TARGET = cockroach
VERSION = 1.0.0
CONFIG -= debug_and_release app_bundle lib_bundle
CONFIG += debug 
PKGCONFIG +=
QT = core gui svg
SOURCES += MainForm.cpp PlotForm.cpp main.cpp
HEADERS += MainForm.h PlotForm.h ftd2xx.h structs.h
FORMS += MainForm.ui PlotForm.ui
RESOURCES +=
TRANSLATIONS +=
OBJECTS_DIR = build/Debug/MinGW-Windows
MOC_DIR = 
RCC_DIR = 
UI_DIR = 
QMAKE_CC = gcc
QMAKE_CXX = g++
DEFINES += 
INCLUDEPATH += ../../../usr/qt5/5.1.0/mingw48_32/include/qnavwidget ../../../usr/qt5/5.1.0/mingw48_32/include/qwt 
LIBS += 
LIBS += -lQNavWidgetd
LIBS += -lqwtd
CONFIG += debug console
QT += widgets
