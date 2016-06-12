/*! \mainpage An improved gradient mesh tool (working title)
 *
 * \section source Source code
 *
 * This documentation presents the source code of the application developed
 * for a research project at HiOA. The application represents a new gradient
 * mesh tool.
 *
 * \section Overview
 *
 * The code structure includes the following namespaces:
 *
 * Ui. Auto-generated classes in Qt.\n
 * GUILogic. Classes that perform logic from user input.
 * They are dependent on underlying model-classes. \n
 * subdivMesh. Mesh class with helper classes. The mesh class is the only
 * class that is to be used outside the namespace. It has been developed in
 * previous projects and is thus an in-house library.
 *
 * The project uses the following external libraries:\n
 * Qt 5.6\n
 * OpenMesh
 *
 */

#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QOpenGLWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
