QT += core gui
QT += xml
QT += core gui network
QT += sql

greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets
#判断是否有websocket模块
qtHaveModule(websockets) {
QT += websockets
DEFINES += websocket
}}

TARGET      = uidemo01
TEMPLATE    = app

HEADERS     += head.h
SOURCES     += main.cpp
RESOURCES   += other/main.qrc
RESOURCES   += $$PWD/../core_qss/qss.qrc

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/form
include ($$PWD/form/form.pri)

INCLUDEPATH += $$PWD/../core_common
include ($$PWD/../core_common/core_common.pri)
