TEMPLATE = app
CONFIG -= app_bundle
QT += widgets

PROJECTROOT = $$PWD/../..

include(src.pri)
LIBS += -lmysqlcppconn

DISTFILES += ui/UIForm.ui.qml ui/UI.qml
