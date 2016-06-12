#include "gmcanvas.h"
#include <QDebug>

GMCanvas::GMCanvas(QWidget *parent) :
    QOpenGLWidget{parent}
{

}

void GMCanvas::initializeGL()
{
    // initialise an OpenGL context
    initializeOpenGLFunctions();
    glWidth = 1200;
    glHeight = 1000;
}

void GMCanvas::resizeGL(int, int)
{
    glOrtho(0, glWidth, 0, glHeight, -1.0, 1.0);
}

void GMCanvas::paintGL()
{    
    // Qt painter, create object on stack
    // for automatic garbage collection (no need to call end())
    QPainter qPainter;

    /******* Start painting with OpenGL ***********/
    qPainter.begin(this);
    qPainter.beginNativePainting();

    // set background colour and clear framebuffer
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw stuff
    meshHandler.drawGLMesh(this);

    /******* Start painting with Qt ***********/
    qPainter.endNativePainting();

    // draw other stuff on top of the OpenGL painting
    qPainter.setPen(Qt::blue);
    qPainter.setFont(QFont("Arial", 30));
    qPainter.drawText(rect(), Qt::AlignCenter, "Qt");
}
