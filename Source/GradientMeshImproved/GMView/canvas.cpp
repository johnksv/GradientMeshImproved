#include "canvas.h"

#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QList>

GMCanvas::GMCanvas(QObject * parent):
    QGraphicsScene(parent)
{
    opengl_ = new GMOpenGLWidget(&meshHandler_);
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
    for(CanvasItemFace* face: item_faces)
    {
        removeItem(face);
        delete face;
    }
    item_faces.clear();

    for(CanvasItemLine* line : item_lines)
    {
        removeItem(line);
        delete line;
    }
    item_lines.clear();

    for(int i = 0; i <item_points.size(); i++)
    {
        removeItem(item_points[i]);
        meshHandler_.removeVertex(i);
        delete item_points[i];
    }
    item_points.clear();
    items_selected.clear();

    update();
}


void GMCanvas::handleFileDialog(QString location, bool import)
{
    if(import)
    {
        qDebug() << "scne import";
        meshHandler_.importGuiMesh(location);
        vector<QPointF> vertices = meshHandler_.vertices();
        for(QPointF point : vertices){
            CanvasItemPoint *item = new CanvasItemPoint(point);
            addItemPoint(item);
        }
    }
    else
    {
        meshHandler_.saveGuiMeshOff(location);
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

void GMCanvas::updateVertexFromPoint(CanvasItemPoint *item, short mode)
{
    int index;
    for (int i =0; i < item_points.size(); i++)
    {
        if(item_points[i] == item){
            index = i;
            i = item_points.size();
        }
    }
    if(mode == 0)
    {
        meshHandler_.setColor(index,item->color());
    }
    else if(mode == 1)
    {
        meshHandler_.setWeight(index,item->weight());
    }
}

void GMCanvas::prepareRendering()
{
    meshHandler_.prepareGuiMeshForSubd();
    opengl_->paintGL();
}

void GMCanvas::setDrawColorVertex(QColor pointColor)
{
    pointColor_ = pointColor;
}

void GMCanvas::handleMousePressVert(QGraphicsSceneMouseEvent *mouseEvent)
{
    bool collide = false;
    int collideWithIndex;

    CanvasItemPoint *itemPoint = new CanvasItemPoint(mouseEvent->scenePos(), pointColor_);
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
            meshHandler_.addVertexFromPoint(itemPoint->position(), pointColor_);
        }

        if(drawMode_ ==drawModeCanvas::vertAndEdge)
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
                    qDebug() << "Collide";
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
                //TODO: Map lines, (start end) so no doubles
                //Check with discontinutiy edges (how to handle them)
                line = new CanvasItemLine(startPoint,endPoint);
                layers_.at(activeLayerIndex_)->addToGroup(line);
                item_lines.push_back(line);
            }
        }
    }
    else
    {
        if(!collide)
        {
            makeFace();
        }
        delete itemPoint;
    }
}

void GMCanvas::addItemPoint(CanvasItemPoint *item)
{
    item->setZValue(2);
    item_points.push_back(item);
    layers_.at(activeLayerIndex_)->addToGroup(item);
    update(item->boundingRect());
}

void GMCanvas::makeFace()
{
    qDebug() << meshHandler_.makeFace();
    CanvasItemFace *face = new CanvasItemFace();
    for(CanvasItemPoint *item : items_selected){
        face->addCanvasPoint(item);
    }
    layers_.at(activeLayerIndex_)->addToGroup(face);
    item_faces.push_back(face);
    items_selected.clear();
    update();
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

void GMCanvas::setActiveLayer(int index)
{
    if(index < 0 || index >= layers_.size())
    {
        index = 0;
    }
    else
    {
        activeLayerIndex_ = index;
    }
}

void GMCanvas::addLayer(QString name)
{
    layers_.push_back(new CanvasItemGroup(name));
    addItem(layers_.back());
}

void GMCanvas::changeLayerName(int index, QString newName)
{
    layers_.at(index)->setText(newName);
}

void GMCanvas::deleteLayer(int index)
{
    CanvasItemGroup *group = layers_.at(index);
    QList<QGraphicsItem*> children = group->childItems();
    for(int i = 0; i < children.size(); i++)
    {
        removeItem(children.at(i));
    }
    layers_.erase(layers_.begin()+index);
    removeItem(group);
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

void GMCanvas::layerChangeIndex(int index, bool moveDown)
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
