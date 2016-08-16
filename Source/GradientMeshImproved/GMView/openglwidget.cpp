#include "openglwidget.h"
#include <QMouseEvent>
#include <QDebug>

using namespace GMView;

GMOpenGLWidget::GMOpenGLWidget(QWidget *parent) : QOpenGLWidget{parent}
{
}

GMOpenGLWidget::GMOpenGLWidget(vector<GUILogic::MeshHandler *> *meshHandlers, QWidget *parent) :
    meshHandlers_(meshHandlers), QOpenGLWidget{parent}
{

}

void GMOpenGLWidget::initializeGL()
{
    // initialise an OpenGL context
    initializeOpenGLFunctions();
    glWidth = 1280;
    glHeight = 640;

}

void GMOpenGLWidget::resizeGL(int, int)
{
}

void GMOpenGLWidget::setMeshHandlers(vector<GUILogic::MeshHandler *> *meshHandlers)
{
    meshHandlers_ = meshHandlers;
}

void GMOpenGLWidget::paintGL()
{
    if(meshHandlers_ != nullptr)
    {
        QPainter qPainter;
        /******* Start painting with OpenGL ***********/
        qPainter.begin(this);
        qPainter.beginNativePainting();

        // set background colour and clear framebuffer
        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glLoadIdentity();
        glOrtho(0, 1200, 600, 0, -1.0, 1.0);

        // draw stuff
        for(GUILogic::MeshHandler *layer : *meshHandlers_)
        {
            layer->drawGLMesh(this);
        }

        /******* Start painting with Qt ***********/
        qPainter.endNativePainting();
    }
}

void GMOpenGLWidget::mousePressEvent(QMouseEvent* event){
    //If i need some MouseEvents on the openGLWidget
}
