TEMPLATE = app
TARGET = ink_coverage
DEPENDPATH += .
INCLUDEPATH += /usr/include/poppler/qt5
QMAKE_LFLAGS += -s
LIBS += -lpoppler-qt5

QT += widgets
CONFIG -= debug_and_release debug


# build dir
UI_DIR  =     ../build
MOC_DIR =     ../build
RCC_DIR =     ../build
OBJECTS_DIR = ../build
mytarget.commands += $${QMAKE_MKDIR} ../build


# Input
HEADERS += $$files(*.h)
SOURCES += $$files(*.cpp)
FORMS += $$files(*.ui)
#RESOURCES += resources.qrc


# install
unix {
    INSTALLS += target
    target.path = /usr/local/bin
}

