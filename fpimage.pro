#-------------------------------------------------
#
# Project created by QtCreator 2013-01-31T10:46:22
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = FPImage
TEMPLATE = app


#OCV INCLUDEPATH +=  C:/Users/alumno/Desktop/opencv31/opencv/build/include/opencv \
#OCV                 C:/Users/alumno/Desktop/opencv31/opencv/build/include

#OCV LIBS += -LC:/Users/alumno/Desktop/opencv31/opencv/build/x64/vc15/lib

# Añadir PATH = C:\Users\alumno\Desktop\opencv31\opencv\build\x64\vc15\bin en panel de control


#OCV CONFIG(debug, debug|release) {
#OCV
#OCV     LIBS+=  -lopencv_world310d
#OCV }


#OCV CONFIG(release, debug|release) {
#OCV
#OCV     LIBS+=  -lopencv_world310
#OCV }


SOURCES += main.cpp\
        fpimage.cpp

HEADERS  += fpimage.h

FORMS    += fpimage.ui
