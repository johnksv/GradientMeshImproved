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
            CanvasItemPoint *item = new CanvasItemPoint(point);
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
    switch(drawMode){
    case drawModeCanvas::vertices:
        handleMousePressVert(mouseEvent);
        break;
    case drawModeCanvas::vertAndEdge:
        handleMousePressVertAndEdge(mouseEvent);
        break;
    case drawModeCanvas::edge:
        break;
    case drawModeCanvas::faces:
        break;
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void GMCanvas::handleMousePressVert(QGraphicsSceneMouseEvent *mouseEvent)
{
    bool collide = false;
    if(mouseEvent->button() != Qt::RightButton)
    {
        CanvasItemPoint *item = new CanvasItemPoint(mouseEvent->scenePos());
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
        }
    }
}

void GMCanvas::handleMousePressVertAndEdge(QGraphicsSceneMouseEvent *mouseEvent)
{
    bool collide = false;
    int collideWithIndex;

    if(mouseEvent->button() != Qt::RightButton)
    {
        CanvasItemPoint *item = new CanvasItemPoint(mouseEvent->scenePos());
        for(int i = 0; i < item_points.size();i++)
        {
            if(item->collidesWithItem(item_points[i])){
                collide = true;
                collideWithIndex = i;
                i = item_points.size();
            }
        }
        if(!collide)
        {
            addItemPoint(item);
        }

        if(item_points.size()>1)
        {
            CanvasItemLine *line;
            CanvasItemPoint* endPoint;
            CanvasItemPoint* startPoint;

            //TODO: more logic, e.g. save first node, last node
            if(collide)
            {

                endPoint = item_points.at(collideWithIndex);
                startPoint = item_points.back();
            }
            else
            {
                endPoint = item_points.back();
                startPoint = item_points.at(item_points.size()-2);
            }

            line = new CanvasItemLine(startPoint,endPoint);
            addItem(line);
        }
    }
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
