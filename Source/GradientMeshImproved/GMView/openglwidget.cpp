#include "openglwidget.h"
#include <QMouseEvent>
#include <QDebug>

GMOpenGLWidget::GMOpenGLWidget(vector<GUILogic::MeshHandler *> *meshHandlers, QWidget *parent) :
    meshHandlers(meshHandlers), QOpenGLWidget{parent}
{

}

void GMOpenGLWidget::initializeGL()
{
    // initialise an OpenGL context
    initializeOpenGLFunctions();
    glWidth = 50;
    glHeight = 50;

}

void GMOpenGLWidget::resizeGL(int, int)
{
    glOrtho(-10, glWidth, -10, glHeight, -1.0, 1.0);
}

void GMOpenGLWidget::paintGL()
{
    QPainter qPainter;
    /******* Start painting with OpenGL ***********/
    qPainter.begin(this);
    qPainter.beginNativePainting();

    // set background colour and clear framebuffer
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw stuff
    for(auto layer : *meshHandlers)
    {
        layer->drawGLMesh(this);
    }

    /******* Start painting with Qt ***********/
    qPainter.endNativePainting();
    qPainter.fillRect(rect(),Qt::blue);
}

void GMOpenGLWidget::mousePressEvent(QMouseEvent* event){
    int x = event->x();
    int y = event->y();
    //If i need some MouseEvents on the openGLWidget
}
