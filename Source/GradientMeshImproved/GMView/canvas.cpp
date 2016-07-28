#include "canvas.h"

#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QList>
#include <QVector4D>
#include <QUndoCommand>

GMCanvas::GMCanvas(QObject * parent):
    QGraphicsScene(parent)
{
    meshHandlers_.push_back(new GUILogic::MeshHandler);
    opengl_ = new GMOpenGLWidget(&meshHandlers_);
    QGraphicsProxyWidget *openGLWidget = addWidget(opengl_);
    openGLWidget->setPos(0,0);
    openGLWidget->setZValue(0);

    CanvasItemGroup *layer = new CanvasItemGroup("Layer 1");
    layers_.push_back(layer);
    addItem(layer);

    //TODO: Change sceneRect(?)
    setSceneRect(itemsBoundingRect());
}

void GMCanvas::clearAll()
{
    for(CanvasItemFace* face: layers_.at(currLayerIndex_)->faces)
    {
        removeItem(face);
        delete face;
    }
    layers_.at(currLayerIndex_)->faces.clear();

    for(CanvasItemLine* line :  layers_.at(currLayerIndex_)->lines)
    {
        removeItem(line);
        delete line;
    }
     layers_.at(currLayerIndex_)->lines.clear();

    for(int i = 0; i < layers_.at(currLayerIndex_)->points.size(); i++)
    {
        CanvasItemPoint *canvasPoint = layers_.at(currLayerIndex_)->points.at(i);
        removeItem(canvasPoint);
        delete  layers_.at(currLayerIndex_)->points.at(i);
    }
     layers_.at(currLayerIndex_)->points.clear();
     layers_.at(currLayerIndex_)->points_selected.clear();


     meshHandlers_.at(currLayerIndex_)->clearAll();
     update();
}

void GMCanvas::resetLineStartEnd()
{
    lineStartPoint_ = nullptr;
    lineEndPoint_ = nullptr;
}


void GMCanvas::handleFileDialog(QString location, bool import)
{
    if(import)
    {
        qDebug() << "scne import";
        meshHandlers_.at(currLayerIndex_)->importGuiMesh(location);
        vector<QVector4D> vertices = meshHandlers_.at(currLayerIndex_)->vertices();
        for(QVector4D vertex : vertices){
            CanvasItemPoint *item = new CanvasItemPoint();
            QPointF point(vertex.x(), vertex.y());
            item->setPos(point);
            item->setVertexHandleIdx(vertex.w());
            addItemPoint(item);
        }

        vector<QVector4D> edges = meshHandlers_.at(currLayerIndex_)->edges();
        for(QVector4D edge : edges){
            //TODO add edge
        }

    }
    else
    {
        meshHandlers_.at(currLayerIndex_)->saveGuiMeshOff(location);
    }
}



void GMCanvas::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    switch(drawMode_){
    case drawModeCanvas::move:
        QGraphicsScene::mousePressEvent(mouseEvent);
        break;
    case drawModeCanvas::vertAndEdge:
        handleMousePressVert(mouseEvent);
        mouseEvent->accept();
        break;
    case drawModeCanvas::edge:
        break;
    case drawModeCanvas::faces:
        break;
    }

}

void GMCanvas::updateVertexFromPoint(CanvasItemPoint &item, short mode)
{
    int vertHanIdx = item.vertexHandleIdx();

    if(mode == 0)
    {
        meshHandlers_.at(currLayerIndex_)->setVertexPoint(vertHanIdx, item.pos());
    }
    else if(mode == 1)
    {
        meshHandlers_.at(currLayerIndex_)->setVertexColor(vertHanIdx,item.color());
    }
    else if(mode == 2)
    {
        meshHandlers_.at(currLayerIndex_)->setVertexWeight(vertHanIdx,item.weight());
    }
    else
    {
        qDebug() << "Canvas.cpp UpdateVertexFromPoint: Illegal mode. No changes made.";
    }
}

void GMCanvas::drawOpenGlOnCanvas(bool drawOnCanvas)
{
        opengl_->setVisible(drawOnCanvas);
}

void GMCanvas::prepareRendering()
{
    meshHandlers_.at(currLayerIndex_)->prepareGuiMeshForSubd();
    opengl_->paintGL();
}

void GMCanvas::setDrawColorVertex(QColor pointColor)
{
    pointColor_ = pointColor;
}

void GMCanvas::handleMousePressVert(QGraphicsSceneMouseEvent *event)
{
    bool madeFace = false;
    bool collide = false;
    int collideWithIndex;

    CanvasItemPoint *itemPoint = new CanvasItemPoint(pointColor_);
    itemPoint->setPos(event->scenePos());

    for(int i = 0; i < layers_.at(currLayerIndex_)->points.size();i++)
    {
        if(itemPoint->collidesWithItem(layers_.at(currLayerIndex_)->points.at(i))){
            collide = true;
            collideWithIndex = i;
            i = layers_.at(currLayerIndex_)->points.size();
        }
    }
    if(event->button() == Qt::LeftButton)
    {
        if(!collide)
        {
            addItemPoint(itemPoint);
            int vertexHandleIdx = meshHandlers_.at(currLayerIndex_)->addVertex(itemPoint->pos(), pointColor_);
            itemPoint->setVertexHandleIdx(vertexHandleIdx);

            //The point is new, and should be added to a new face.
            vertesToAddFace_.push_back(vertexHandleIdx);
        }
        else
        {
            int vertexHandleIdx = layers_.at(currLayerIndex_)->points.at(collideWithIndex)->vertexHandleIdx();
            if (vertesToAddFace_.size() == 0)
            {
                //Face start at an already existing vertex
                vertesToAddFace_.push_back(vertexHandleIdx);
            }
            else
            {
                //Check if first element has edges already connect to it. If not, the point is already added.
                if(meshHandlers_.at(currLayerIndex_)->vertexValence(vertesToAddFace_.at(0)) > 0)
                {
                    vertesToAddFace_.push_back(vertexHandleIdx);
                }

                //Else: Make a face of the points added. (E.g. the bounadary).
                meshHandlers_.at(currLayerIndex_)->makeFace(vertesToAddFace_);



                //For debugging purposes
                CanvasItemFace * face = new CanvasItemFace();
                layers_.at(currLayerIndex_)->faces.push_back(face);
                for(int handle : vertesToAddFace_)
                {
                    for(CanvasItemPoint *point : layers_.at(currLayerIndex_)->points)
                    {
                        if(point->vertexHandleIdx() == handle)
                        {
                            face->addCanvasPoint(point);
                        }
                    }
                }
                layers_.at(currLayerIndex_)->addToGroup(face); //End debugging purposes


                madeFace = true;
                vertesToAddFace_.clear();
            }
        }

       if(lineStartPoint_ == nullptr){
           if(collide){
                lineStartPoint_ = layers_.at(currLayerIndex_)->points.at(collideWithIndex);
           }else{
               lineStartPoint_ = itemPoint;
           }
       }else{
           if(collide) {
                lineEndPoint_ = layers_.at(currLayerIndex_)->points.at(collideWithIndex);
           }else{
               lineEndPoint_ = itemPoint;
           }
       }

       if(lineStartPoint_ != nullptr && lineEndPoint_ != nullptr)
       {
           CanvasItemLine *line = new CanvasItemLine(lineStartPoint_, lineEndPoint_);

           //Check if line exists
           bool exists = false;
           int size = layers_.at(currLayerIndex_)->lines.size();
           for(int i = 0; i < size; i++ )
           {
               if(*(layers_.at(currLayerIndex_)->lines.at(i)) == *line	||
                        ( layers_.at(currLayerIndex_)->lines.at(i)->endPoint() == line->startPoint() &&
                          layers_.at(currLayerIndex_)->lines.at(i)->startPoint() == line->endPoint()))
               {
                    exists = true;
                    i = size;
               }
           }//End check line exists

           if(exists)
           {
              //Delete line from heap
              delete line;
           }
           else
           {
               layers_.at(currLayerIndex_)->addToGroup(line);
               layers_.at(currLayerIndex_)->lines.push_back(line);
           }

           lineStartPoint_ = lineEndPoint_;
           lineEndPoint_ = nullptr;
       }


       //Startpoint should be reset if a face was made.
       if(madeFace) lineStartPoint_ = nullptr;

       //Should not delete ItemPoint from heap.
       return;
    }

    delete itemPoint;
}

void GMCanvas::addItemPoint(CanvasItemPoint *item)
{
    item->setZValue(2);
    layers_.at(currLayerIndex_)->points.push_back(item);
    layers_.at(currLayerIndex_)->addToGroup(item);
    update(item->boundingRect());
}

void GMCanvas::makeFace()
{
    //qDebug() << meshHandlers_.at(currLayerIndex_)->makeFace(0);
}


void GMCanvas::setRenderingMode(int mode){
    renderingMode_ = mode;
}
void GMCanvas::setDrawingMode(drawModeCanvas drawMode){
    resetLineStartEnd();
    this->drawMode_ = drawMode;
}

vector<CanvasItemGroup *> GMCanvas::layers()
{
    return layers_;
}

vector<GUILogic::MeshHandler *> *GMCanvas::meshHandlers()
{
    return &meshHandlers_;
}

void GMCanvas::setActiveLayer(unsigned char index)
{
    if(currLayerIndex_ < layers_.size()) makeFace();

    if(index < 0 || index >= layers_.size())
    {
        currLayerIndex_ = 0;
    }
    else
    {
        currLayerIndex_ = index;
    }
    layers_.at(currLayerIndex_)->points_selected.clear();
}

void GMCanvas::addLayer(QString name)
{
    layers_.push_back(new CanvasItemGroup(name));
    addItem(layers_.back());
    meshHandlers_.push_back(new GUILogic::MeshHandler);
}

void GMCanvas::deleteLayer(int index)
{
    if(layers_.size() > 1)
    {
        CanvasItemGroup *group = layers_.at(index);
        QList<QGraphicsItem*> children = group->childItems();
        for(int i = 0; i < children.size(); i++)
        {
            removeItem(children.at(i));
        }
        layers_.erase(layers_.begin()+index);
        removeItem(group);

        delete meshHandlers_.at(index);
        meshHandlers_.erase(meshHandlers_.begin()+index);
        setActiveLayer(layers_.size()-1);
    }
}

void GMCanvas::toogleLayerVisibility(int index)
{
    CanvasItemGroup *selectedLayer = layers_.at(index);
    if(selectedLayer->isVisible())
    {
        selectedLayer->hide();
    }
    else
    {
        selectedLayer->show();
    }
}
