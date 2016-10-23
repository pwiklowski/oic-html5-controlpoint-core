#-------------------------------------------------
#
# Project created by QtCreator 2015-01-06T13:13:43
#
#-------------------------------------------------

QT       += core network script websockets
QT       += serialport
QT       -= gui

CONFIG += c++11

TARGET = oic_cp_core
CONFIG   += console
CONFIG   -= app_bundle


TEMPLATE = app




#Uncomment for RaspberryPI
#LIBS += -lwiringPi


LIBS += -L../../liboic -loic
INCLUDEPATH += ../../liboic

LIBS += -L../../libcoap -lcoap
INCLUDEPATH += ../../libcoap

INCLUDEPATH += ../../std


SOURCES += main.cpp \
    SmartHomeServer.cpp \
    Device.cpp \
    OcfDeviceController.cpp \
    IotDevice.cpp \
    IPv4OcfDeviceController.cpp \
    WebSocketServer.cpp

HEADERS += \
    SmartHomeServer.h \
    Device.h \
    OcfDeviceController.h \
    IotDevice.h \
    IPv4OcfDeviceController.h \
    WebSocketServer.h
