#include "openglwidget.h"
#include <QMouseEvent>
#include <QDebug>
#include <QGraphicsView>

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
}

void GMOpenGLWidget::resizeGL(int, int)
{
}

void GMOpenGLWidget::setScene(GMCanvas *scene)
{
    scene_ = scene;
}

void GMOpenGLWidget::setRenderMode(renderModeOpenGL mode)
{
    renderMode_ = mode;
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
    QRectF boundingRect;

    if(renderMode_ == renderModeOpenGL::boundingRect)
    {
        boundingRect = scene_->itemsBoundingRect();
    }
    else if(renderMode_ == renderModeOpenGL::viewportRender)
    {
        if(scene_->views().size() != 0)
        {
            QGraphicsView * view = scene_->views().first();
            QRectF rect         = view->rect();
            QPointF topLeft     = view->mapToScene(rect.x(), rect.y());
            QPointF bottomRight = view->mapToScene( rect.width(), rect.height());
            boundingRect        = QRectF( topLeft, bottomRight );
        }
        else
        {
            boundingRect = scene_->itemsBoundingRect();
        }
    }
    else if(renderMode_ == renderModeOpenGL::fullSceneRender)
    {
        //if fixedWidht = sceneRect this will display correct as backrgound, but it will be very slow
        boundingRect = scene_->sceneRect();
    }

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
    update();
}
