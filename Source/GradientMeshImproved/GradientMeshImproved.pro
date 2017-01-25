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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../OpenMesh/lib/ -lOpenMeshCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../OpenMesh/lib/ -lOpenMeshCored

INCLUDEPATH += $$PWD/../OpenMesh/include
DEPENDPATH += $$PWD/../OpenMesh/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../OpenMesh/lib/libOpenMeshCore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../OpenMesh/lib/libOpenMeshCored.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../OpenMesh/lib/OpenMeshCore.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../OpenMesh/lib/OpenMeshCored.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../OpenMesh/lib/ -lOpenMeshTools
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../OpenMesh/lib/ -lOpenMeshToolsd

INCLUDEPATH += $$PWD/../OpenMesh/include
DEPENDPATH += $$PWD/../OpenMesh/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../OpenMesh/lib/libOpenMeshTools.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../OpenMesh/lib/libOpenMeshToolsd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../OpenMesh/lib/OpenMeshTools.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../OpenMesh/lib/OpenMeshToolsd.lib
