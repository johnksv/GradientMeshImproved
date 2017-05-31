#-------------------------------------------------
#
# Project created by QtCreator 2016-06-11T11:20:24
#
#-------------------------------------------------

#ENTER YOUR PATH TO OPENMESH INSTALLATION ( http://www.openmesh.org/ )
OPENMESH = $$PWD/../OpenMesh/lib

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
    GMView/customgraphicsview.cpp \
    GMView/canvas.cpp \
    GMView/openglwidget.cpp \
    GMView/canvasitemline.cpp \
    GMView/canvaspoint.cpp \
    GMView/canvasitemgroup.cpp \
    GMView/canvaspointconstraint.cpp \
    GMView/canvasitemface.cpp \
    undoCommands/guichange.cpp


HEADERS  += mainwindow.h \
    subdivMesh/mesh.h \
    subdivMesh/point_3d.h \
    subdivMesh/spline.h \
    subdivMesh/tostring.h \
    subdivMesh/types.h \
    GUILogic/meshhandler.h \
    subdivMesh/utils.h \
    customMeshing/openmeshext.h \
    GMView/customgraphicsview.h \
    GMView/canvas.h \
    GMView/openglwidget.h \
    GMView/canvasitemline.h \
    GMView/canvaspoint.h \
    GMView/canvasitemgroup.h \
    GMView/canvaspointconstraint.h \
    GMView/utils.h \
    GMView/canvasitemface.h \
    undoCommands/guichange.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OPENMESH/ -lOpenMeshCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OPENMESH/ -lOpenMeshCored

INCLUDEPATH += $$PWD/../OpenMesh/include
DEPENDPATH += $$PWD/../OpenMesh/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OPENMESH/libOpenMeshCore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OPENMESH/libOpenMeshCored.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OPENMESH/OpenMeshCore.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OPENMESH/OpenMeshCored.lib

win32:CONFIG(release, debug|release): LIBS += -L$$OPENMESH/ -lOpenMeshTools
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OPENMESH/ -lOpenMeshToolsd

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OPENMESH/libOpenMeshTools.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OPENMESH/libOpenMeshToolsd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OPENMESH/OpenMeshTools.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OPENMESH/OpenMeshToolsd.lib
