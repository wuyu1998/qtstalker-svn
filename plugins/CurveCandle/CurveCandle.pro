TEMPLATE = lib
CONFIG += plugin
MOC_DIR += build
OBJECTS_DIR += build
INCLUDEPATH += ../../src


HEADERS += CurveCandle.h
SOURCES += CurveCandle.cpp
HEADERS += CurveCandleDialog.h
SOURCES += CurveCandleDialog.cpp
HEADERS += CurveCandleObject.h
SOURCES += CurveCandleObject.cpp
HEADERS += ../../src/Dialog.h
HEADERS += ../../src/Object.h
HEADERS += ../../src/Util.h
HEADERS += ../../src/Plugin.h
HEADERS += ../../src/PluginCommand.h
HEADERS += ../../src/ObjectCommand.h

OBJECTS += ../../src/build/Dialog.o
OBJECTS += ../../src/build/Object.o
OBJECTS += ../../src/build/ObjectCommand.o
OBJECTS += ../../src/build/Util.o


HOME=$$system(echo $HOME) 
target.path =  $${HOME}/OTA/lib
INSTALLS += target

QT += core
QT += gui
