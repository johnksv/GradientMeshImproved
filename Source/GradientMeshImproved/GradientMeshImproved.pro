#-------------------------------------------------
#
# Project created by QtCreator 2016-06-11T11:20:24
#
#-------------------------------------------------

#ENTER YOUR PATH TO OPENMESH INSTALLATION ( http://www.openmesh.org/ )
#Required: OpenMesh 6.3
OPENMESH = "D:/Program Files/OpenMesh"

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

win32:CONFIG(release, debug|release): LIBS += -L$$OPENMESH/lib/ -lOpenMeshCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OPENMESH/lib/ -lOpenMeshCored

INCLUDEPATH += $$OPENMESH/include
DEPENDPATH += $$OPENMESH/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OPENMESH/lib/libOpenMeshCore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OPENMESH/lib/libOpenMeshCored.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OPENMESH/lib/OpenMeshCore.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OPENMESH/lib/OpenMeshCored.lib

win32:CONFIG(release, debug|release): LIBS += -L$$OPENMESH/lib/ -lOpenMeshTools
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OPENMESH/lib/ -lOpenMeshToolsd

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OPENMESH/lib/libOpenMeshTools.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OPENMESH/lib/libOpenMeshToolsd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OPENMESH/lib/OpenMeshTools.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OPENMESH/lib/OpenMeshToolsd.lib


test {
    QT += testlib
    TARGET = GradientMeshImproved-Test
    SOURCES -= main.cpp

    #HEADERS += \


    SOURCES += test/main.cpp


   message(Test build)
}
