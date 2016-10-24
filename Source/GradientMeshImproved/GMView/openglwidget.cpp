#include "openglwidget.h"
#include <QMouseEvent>
#include <QDebug>

using namespace GMView;

GMOpenGLWidget::GMOpenGLWidget(QWidget *parent) : QOpenGLWidget{parent}
{
}

GMOpenGLWidget::GMOpenGLWidget(GMCanvas *scene, QWidget *parent) :
     scene_(scene), QOpenGLWidget{parent}
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

void GMOpenGLWidget::setScene(GMCanvas *scene)
{
    scene_ = scene;
}

void GMOpenGLWidget::paintGL()
{

    vector<GUILogic::MeshHandler *> meshHandlers;
    if (scene_->multiResMeshHandlers()->empty())
    {
        meshHandlers = *scene_->meshHandlers();
    }
    else
    {
        meshHandlers = *scene_->multiResMeshHandlers();
    }
    QRectF boundingRect = scene_->sceneRect();

    QPainter qPainter;
    /******* Start painting with OpenGL ***********/
    qPainter.begin(this);
    qPainter.beginNativePainting();

    // set background colour and clear framebuffer
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity();
    glOrtho(boundingRect.left(),boundingRect.right() ,boundingRect.bottom(),boundingRect.top(), -1.0, 1.0);

    // draw stuff
    for (int i = 0; i < meshHandlers.size(); ++i)
    {
        meshHandlers.at(i)->drawGLMesh(this);
    }

    /******* Start painting with Qt ***********/
    qPainter.endNativePainting();
    qPainter.end();

}

void GMOpenGLWidget::mousePressEvent(QMouseEvent* event){
    //If i need some MouseEvents on the openGLWidget
}
