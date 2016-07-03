#include "gmcanvas.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>


GMCanvas::GMCanvas(QObject * parent):
    QGraphicsScene(parent)
{

}

GMCanvas::~GMCanvas()
{

}


void GMCanvas::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    qDebug() << "Over";
}

void GMCanvas::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    qDebug() << "Over" << mouseEvent->scenePos().x() << ","<<mouseEvent->scenePos().y();
}
