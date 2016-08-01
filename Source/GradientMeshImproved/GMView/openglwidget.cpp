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
    glLoadIdentity(); //Works with mesh.cpp if line uncommented, but will not then draw polygon (line 50)
    glOrtho(0, glWidth, glHeight, 0, -1.0, 1.0);
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

        glBegin(GL_POLYGON);
        glColor3f(0,1,0);
        glVertex2f(0,0);
        glVertex2f(.5f , .5f);
        glColor3f(0,0,1);
        glVertex2f(0.5f,0);

        glEnd();



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
    int x = event->x();
    int y = event->y();
    qDebug() << "OpenGL click: " << event->pos();
    //If i need some MouseEvents on the openGLWidget
}
