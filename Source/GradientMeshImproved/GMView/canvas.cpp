#include "canvas.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QList>
#include <QObjectList>


GMCanvas::GMCanvas(QObject * parent):
    QGraphicsScene(parent)
{
    opengl = new GMOpenGLWidget();
    QGraphicsProxyWidget *openGLWidget = addWidget(opengl);
    openGLWidget->setPos(0,0);
    openGLWidget->setZValue(0);
    setSceneRect(itemsBoundingRect());
}


void GMCanvas::handleFileDialog(QString location, bool import)
{
    if(import)
    {
        meshHandler.importGuiMesh(location);
    }
    else
    {
        meshHandler.saveGuiMeshOff(location);
    }
}



void GMCanvas::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    bool collide = false;

    if(mouseEvent->button() != Qt::RightButton)
    {
        qDebug() << "Pos:" << mouseEvent->scenePos().x() << ","<<mouseEvent->scenePos().y();
        GMCanvasItem *item = new GMCanvasItem(nullptr, mouseEvent->scenePos());
        for(int i = 0; i < item_points.size();i++)
        {
            if(item->collidesWithItem(item_points[i])){
                collide = true;
                i = item_points.size();
            }
        }
        if(!collide)
        {
            item->setZValue(1);
            item_points.push_back(item);
            addItem(item);
            update(item->boundingRect());
            qDebug() << "Added point";
        }
    }
    setSceneRect(itemsBoundingRect());
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void GMCanvas::setRenderingMode(int mode){
    renderingMode = mode;
}
void GMCanvas::setDrawingMode(drawModeCanvas drawMode){
    this->drawMode = drawMode;
}
