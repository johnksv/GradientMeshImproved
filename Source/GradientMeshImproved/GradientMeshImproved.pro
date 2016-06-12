#-------------------------------------------------
#
# Project created by QtCreator 2016-06-11T11:20:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GradientMeshImproved
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    subdivMesh/mesh.cpp \
    subdivMesh/point_3d.cpp \
    subdivMesh/spline.cpp \
    GUILogic/meshhandler.cpp \
    gmcanvas.cpp

HEADERS  += mainwindow.h \
    subdivMesh/mesh.h \
    subdivMesh/point_3d.h \
    subdivMesh/spline.h \
    subdivMesh/tostring.h \
    subdivMesh/types.h \
    GUILogic/meshhandler.h \
    subdivMesh/utils.h \
    gmcanvas.h \
    customMeshing/openmeshext.h

FORMS    += mainwindow.ui

####### EXTERAL LIBRARIES ADDED VIA THE "ADD LIBRARY FUNCTIONALITY" #######
####### REMOVE OR ADJUST ACCORDING TO YOUR SETUP!                   #######

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../../../../Program Files/OpenMesh 6.1/lib/' -lOpenMeshCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../../../../Program Files/OpenMesh 6.1/lib/' -lOpenMeshCored

INCLUDEPATH += $$PWD/'../../../../../../../Program Files/OpenMesh 6.1/include'
DEPENDPATH += $$PWD/'../../../../../../../Program Files/OpenMesh 6.1/include'

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../../../../Program Files/OpenMesh 6.1/lib/' -lOpenMeshTools
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../../../../Program Files/OpenMesh 6.1/lib/' -lOpenMeshToolsd

INCLUDEPATH += $$PWD/'../../../../../../../Program Files/OpenMesh 6.1/include'
DEPENDPATH += $$PWD/'../../../../../../../Program Files/OpenMesh 6.1/include'
