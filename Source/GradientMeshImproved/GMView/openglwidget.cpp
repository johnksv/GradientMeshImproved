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

    vector<GUILogic::MeshHandler *> *meshHandle;
    if (scene_->multiResMeshHandlers()->empty())
    {
        meshHandle = scene_->meshHandlers();
    }
    else
    {
        meshHandle = scene_->multiResMeshHandlers();
    }

    QRectF boundingRect = scene_->itemsBoundingRect();

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
    for(GUILogic::MeshHandler *layer : *meshHandle)
    {
        layer->drawGLMesh(this);
    }

    /******* Start painting with Qt ***********/
    qPainter.endNativePainting();

}

void GMOpenGLWidget::mousePressEvent(QMouseEvent* event){
    //If i need some MouseEvents on the openGLWidget
}
