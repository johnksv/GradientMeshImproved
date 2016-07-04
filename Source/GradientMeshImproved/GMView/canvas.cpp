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
        qDebug() << "scne import";
        meshHandler.importGuiMesh(location);
        vector<QPointF> vertices = meshHandler.getVertices();
        for(QPointF point : vertices){
            CanvasItemPoint *item = new CanvasItemPoint(nullptr,point);
            addItemPoint(item);
        }
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
        CanvasItemPoint *item = new CanvasItemPoint(nullptr, mouseEvent->scenePos());
        for(int i = 0; i < item_points.size();i++)
        {
            if(item->collidesWithItem(item_points[i])){
                collide = true;
                i = item_points.size();
            }
        }
        if(!collide)
        {
            addItemPoint(item);
            qDebug() << "Added point";
        }
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void GMCanvas::addItemPoint(CanvasItemPoint *item)
{
    item->setZValue(1);
    item_points.push_back(item);
    addItem(item);
    update(item->boundingRect());
}

void GMCanvas::setRenderingMode(int mode){
    renderingMode = mode;
}
void GMCanvas::setDrawingMode(drawModeCanvas drawMode){
    this->drawMode = drawMode;
}
