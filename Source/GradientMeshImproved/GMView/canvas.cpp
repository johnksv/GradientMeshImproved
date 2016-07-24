#include "canvas.h"

#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QList>

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
        meshHandlers_.at(currLayerIndex_)->removeVertex(canvasPoint->vertexHandleIdx());
        delete  layers_.at(currLayerIndex_)->points.at(i);
    }
     layers_.at(currLayerIndex_)->points.clear();
     layers_.at(currLayerIndex_)->points_selected.clear();

    update();
}


void GMCanvas::handleFileDialog(QString location, bool import)
{
    if(import)
    {
        qDebug() << "scne import";
        meshHandlers_.at(currLayerIndex_)->importGuiMesh(location);
        vector<QPointF> vertices = meshHandlers_.at(currLayerIndex_)->vertices();
        for(QPointF point : vertices){
            CanvasItemPoint *item = new CanvasItemPoint();
            item->setPos(point);
            addItemPoint(item);
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

void GMCanvas::updateVertexFromPoint(CanvasItemPoint &item, short mode)
{
    int index;
    for (int i =0; i < layers_.at(currLayerIndex_)->points.size(); i++)
    {
        if(layers_.at(currLayerIndex_)->points.at(i) == &item){
            index = i;
            i = layers_.at(currLayerIndex_)->points.size();
        }
    }

    if(mode == 0)
    {
        meshHandlers_.at(currLayerIndex_)->setVertexPoint(index, item.pos());
    }
    else if(mode == 1)
    {
        meshHandlers_.at(currLayerIndex_)->setVertexColor(index,item.color());
    }
    else if(mode == 2)
    {
        meshHandlers_.at(currLayerIndex_)->setVertexWeight(index,item.weight());
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
        }

       if(lineStartPoint_ == nullptr)
       {
           if(collide)
           {
                lineStartPoint_ = layers_.at(currLayerIndex_)->points.at(collideWithIndex);
           }
           else
           {
               lineStartPoint_ = itemPoint;
           }
       }
       else
       {
           if(collide)
           {
                lineEndPoint_ = layers_.at(currLayerIndex_)->points.at(collideWithIndex);
           }
           else
           {
               lineEndPoint_ = itemPoint;
           }
       }

       if(lineStartPoint_ != nullptr && lineEndPoint_ != nullptr)
       {
           CanvasItemLine *line = new CanvasItemLine(lineStartPoint_, lineEndPoint_);
           bool exists = false;
           int size = layers_.at(currLayerIndex_)->lines.size();
           for(int i = 0; i < size; i++ )
           {
               if(*(layers_.at(currLayerIndex_)->lines.at(i)) == *line )
               {
                    exists = true;
                    i = size;
               }
           }
           if(exists)
           {
              delete line;
           }
           else
           {
               layers_.at(currLayerIndex_)->addToGroup(line);
               layers_.at(currLayerIndex_)->lines.push_back(line);
               int edgeIdx = meshHandlers()->at(currLayerIndex_)->addEdge(lineStartPoint_->vertexHandleIdx(),
                                                                          lineEndPoint_->vertexHandleIdx());
               line->setEdgeHandleIdx(edgeIdx);
           }

           lineStartPoint_ = lineEndPoint_;
           lineEndPoint_ = nullptr;
       }

        return;
    }
    else if(event->button() == Qt::RightButton)
    {
        if(!collide)
        {
            makeFace();
        }
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
    qDebug() << "TODO: Reimplement makeFace";
}


void GMCanvas::setRenderingMode(int mode){
    renderingMode_ = mode;
}
void GMCanvas::setDrawingMode(drawModeCanvas drawMode){
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

void GMCanvas::layerMoveIndex(int index, bool moveDown)
{
    if(moveDown)
    {
        if(! (index >= layers_.size()))
        {
            std::swap(layers_[index], layers_[index+1]);
        }
    }
    else
    {
        if(! (index <= 0))
        {
            std::swap(layers_.at(index), layers_.at(index-1));
        }
    }
}
