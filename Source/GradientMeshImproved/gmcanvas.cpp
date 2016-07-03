#include "gmcanvas.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>


GMCanvas::GMCanvas(QObject * parent):
    QGraphicsScene(parent)
{
    mainCanvasItem = new GMCanvasItem();

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
    this->addItem(item);
    update(-1000,-1000,2000,2000);
    QGraphicsScene::mousePressEvent(mouseEvent);
}
