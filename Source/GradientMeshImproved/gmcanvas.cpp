#include "gmcanvas.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>


GMCanvas::GMCanvas(QObject * parent):
    QGraphicsScene(parent)
{
    mainCanvasItem = new GMCanvasItem();
    opengl = new GMOpenGLWidget();
    QGraphicsProxyWidget *openGLWidget = addWidget(opengl);
    openGLWidget->setPos(-300,-300);
    openGLWidget->setZValue(0);
}

GMCanvas::~GMCanvas()
{
    delete mainCanvasItem;
}


void GMCanvas::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

}

void GMCanvas::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    qDebug() << "Add:" << mouseEvent->scenePos().x() << ","<<mouseEvent->scenePos().y();
    GMCanvasItem *item = new GMCanvasItem(nullptr, mouseEvent->scenePos());
    item->setZValue(10);
    addItem(item);
    update(-1000,-1000,2000,2000);
    QGraphicsScene::mousePressEvent(mouseEvent);
}
