#include "canvas.h"
#include "canvasitemface.h"
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
        vector<QPointF> vertices = meshHandler.vertices();
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
    case drawModeCanvas::vertAndEdge:
        handleMousePressVert(mouseEvent);
        break;
    case drawModeCanvas::edge:
        break;
    case drawModeCanvas::faces:
        break;
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void GMCanvas::updateColorVertex(CanvasItemPoint *item)
{
    int index;
    for (int i =0; i < item_points.size(); i++)
    {
        if(item_points[i] == item){
            index = i;
            i = item_points.size();
        }
    }

    QVector3D color = QVector3D(item->color().redF(), item->color().greenF(), item->color().blueF());
    meshHandler.setColor(index,color);
}

void GMCanvas::handleMousePressVert(QGraphicsSceneMouseEvent *mouseEvent)
{
    bool collide = false;
    int collideWithIndex;

    CanvasItemPoint *itemPoint = new CanvasItemPoint(mouseEvent->scenePos());
    for(int i = 0; i < item_points.size();i++)
    {
        if(itemPoint->collidesWithItem(item_points[i])){
            collide = true;
            collideWithIndex = i;
            i = item_points.size();
        }
    }

    if(mouseEvent->button() != Qt::RightButton)
    {
        if(!collide)
        {
            addItemPoint(itemPoint);
            meshHandler.addVertexFromPoint(itemPoint->position());
        }

        if(drawMode ==drawModeCanvas::vertAndEdge)
        {
            CanvasItemLine *line;
            CanvasItemPoint* endPoint;
            CanvasItemPoint* startPoint;

            if(collide)
            {
                items_selected.push_back(item_points.at(collideWithIndex));

                if(items_selected.size()<=1)
                {
                    startPoint = items_selected.at(0);
                }
                else
                {
                    startPoint = items_selected.end()[-2];
                    endPoint = items_selected.back();
                }
            }
            else
            {
                items_selected.push_back(item_points.back());
                if(items_selected.size() > 1)
                {
                    startPoint = items_selected.end()[-2];
                    endPoint = items_selected.back();
                }
            }
            if(items_selected.size() > 1)
            {
                line = new CanvasItemLine(startPoint,endPoint);

                addItem(line);
            }
        }

        clearSelection();
    }
    else
    {
        if(collide)
        {

        }
        else
        {
            qDebug() << meshHandler.makeFace();
            CanvasItemFace *face = new CanvasItemFace();
            for(CanvasItemPoint *item : items_selected){
                face->addCanvasPoint(item);
            }
            addItem(face);
            items_selected.clear();
            update();
        }
    }
}

void GMCanvas::addItemPoint(CanvasItemPoint *item){
    item->setZValue(2);
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
