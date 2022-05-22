
TEMPLATE = app
TARGET = ink_coverage
DEPENDPATH += .
INCLUDEPATH += /usr/include/poppler/qt5
LIBS += -lpoppler-qt5

MOC_DIR = build
RCC_DIR = build
UI_DIR = build
OBJECTS_DIR = build
mytarget.commands += $${QMAKE_MKDIR} build

QT += widgets

# Input
HEADERS += main.h
SOURCES += main.cpp

FORMS += mainwindow.ui
#RESOURCES += resources.qrc

# install
INSTALLS += target
target.path = /usr/local/bin

