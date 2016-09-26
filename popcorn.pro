equals(QT_MAJOR_VERSION, 4) {
QT       += core gui network webkit sql
}

equals(QT_MAJOR_VERSION, 5) {
QT       += core gui network webkit sql widgets webkitwidgets multimedia
}

macx {
#QMAKE_MACOSX_DEPLOYMENT_TARGET = $$(MY_OSX_VERSION)
#QMAKE_MAC_SDK = macosx$$(MY_OSX_VERSION)
ICON=popcorn.icns
#QMAKE_LFLAGS += -F/System/Library/Frameworks/ApplicationServices.framework
LIBS += -framework ApplicationServices
}

message(Current Qt Version: $${QT_MAJOR_VERSION})

TARGET = popcorn
TEMPLATE = app
RC_FILE = popcorn.rc


SOURCES += main.cpp\
        mainwindow.cpp \
    jsapi.cpp \
    message.cpp \
    server.cpp \
    client.cpp \
    process_manager.cpp \
    optionsdialog.cpp \
    downloader.cpp \
    database.cpp \
    randomng.c

HEADERS  += \
    mainwindow.h \
    jsapi.h \
    message.h \
    server.h \
    client.h \
    optionsdialog.h \
    downloader.h \
    database.h \
    process_manager.h


RESOURCES += \
    resources.qrc

linux-g++* {
  LIBS += -L/usr/include/X11 -L/usr/include/X11/extensions -lXss -lX11
}

win32 {
    LIBS += -lpsapi
}

FORMS += \
    optionsdialog.ui
