
TEMPLATE = app
TARGET = ink_coverage
DEPENDPATH += .
INCLUDEPATH += /usr/include/poppler/qt4
LIBS += -lpoppler-qt4
MOC_DIR = build
RCC_DIR = build
UI_DIR = build
OBJECTS_DIR = build
mytarget.commands += $${QMAKE_MKDIR} build

# Input
HEADERS += main.h
SOURCES += main.cpp

FORMS += mainwindow.ui
#RESOURCES += resources.qrc

# install
INSTALLS += target
target.path = /usr/local/bin

