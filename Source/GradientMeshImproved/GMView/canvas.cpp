#include "canvas.h"

#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QList>
#include <QVector4D>
#include <QUndoCommand>

using namespace GMView;

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

GMCanvas::~GMCanvas()
{
    for(GUILogic::MeshHandler *mesh : meshHandlers_)
    {
        delete mesh;
    }
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
        //x,y,z = position, w = vertexHandleIdx
        vector<QVector4D> vertices = meshHandlers_.at(currLayerIndex_)->vertices();
        for(QVector4D vertex : vertices){
            CanvasItemPoint *item = new CanvasItemPoint();
            QPointF point(vertex.x(), vertex.y());
            item->setPos(point);
            item->setVertexHandleIdx(vertex.w());
            addControlPoint(item);
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
    case drawModeCanvas::lineTool:
        mouseLineTool(mouseEvent);
        mouseEvent->accept();
        break;
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

void GMCanvas::setRenderConstraintHandlers(bool value)
{
    renderConstraintHandlers_ = value;
}

drawModeCanvas GMCanvas::drawingMode() const
{
    return drawMode_;
}

renderModeCanvas GMCanvas::renderMode() const
{
    return renderingMode_;
}

bool GMCanvas::renderConstraintHandlers() const
{
    return renderConstraintHandlers_;
}

void GMCanvas::mouseLineTool(QGraphicsSceneMouseEvent *event)
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
            addControlPoint(itemPoint);
            int vertexHandleIdx = meshHandlers_.at(currLayerIndex_)->addVertex(itemPoint->pos(), pointColor_);
            itemPoint->setVertexHandleIdx(vertexHandleIdx);

            //The point is new, and should be added to a new face.
            vertsToAddFace_.push_back(itemPoint);
        }
        else
        {
            CanvasItemPoint *collidePoint = layers_.at(currLayerIndex_)->points.at(collideWithIndex);
            if (vertsToAddFace_.size() == 0)
            {
                //Face start at an already existing vertex
                vertsToAddFace_.push_back(collidePoint);
            }
            else
            {
                //Check if first element has edges already connect to it. If not, the point is already added in prev face.
                if(meshHandlers_.at(currLayerIndex_)->vertexValence(vertsToAddFace_.at(0)->vertexHandleIdx()) > 0)
                {
                    vertsToAddFace_.push_back(collidePoint);
                }

                vector<int> vertesToAddFaceIdx;
                //If the face to be should be added inside an already existing face
                bool faceInsideFace = false;
                for (int i = 0; i < vertsToAddFace_.size(); i++)
				{
                    CanvasItemPoint *itemPoint = vertsToAddFace_.at(i);
                    vertesToAddFaceIdx.push_back(itemPoint->vertexHandleIdx());
					//No need to check last or first item,.
                    if (i != 0 && i < vertsToAddFace_.size() - 1)
					{
						for(QGraphicsItem *item: itemPoint->collidingItems())
						{
							if(dynamic_cast<CanvasItemFace*>(item))
							{
								faceInsideFace = true;
							}
						}
					}
                }
                if (vertsToAddFace_.size() == 2) faceInsideFace = true;
                meshHandlers_.at(currLayerIndex_)->makeFace(vertesToAddFaceIdx, faceInsideFace);

                //For debugging purposes
                CanvasItemFace * face = new CanvasItemFace();
                layers_.at(currLayerIndex_)->faces.push_back(face);

                for(int handle : vertesToAddFaceIdx)
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
                vertsToAddFace_.clear();
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

               //TODO:Fix Linker error in VS Community Update 3, but not QTCreator...
               CanvasPointConstraint *startConstraint = new CanvasPointConstraint(lineStartPoint_, line);
               startConstraint->setPos(QPointF(line->line().dx()*0.2, line->line().dy()*0.2));

               CanvasPointConstraint *endConstraint = new CanvasPointConstraint(lineEndPoint_, line);
               endConstraint->setPos(QPointF(line->line().dx()*-0.2, line->line().dy()*-0.2));
           }
           lineStartPoint_ = lineEndPoint_;
           lineEndPoint_ = nullptr;
       }
       //Startpoint should be reset if a face was made.
       if(madeFace) lineStartPoint_ = nullptr;

       //Should not delete ItemPoint from heap.
       return;
    }
    //No use for item. Delete it from heap.
    delete itemPoint;
}

void GMCanvas::addControlPoint(CanvasItemPoint *item)
{
    item->setZValue(2);
    layers_.at(currLayerIndex_)->points.push_back(item);
    layers_.at(currLayerIndex_)->addToGroup(item);
    update(item->boundingRect());
}

void GMCanvas::setRenderingMode(renderModeCanvas mode){
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

GUILogic::MeshHandler *GMCanvas::currentMeshHandler()
{
    return meshHandlers_.at(currLayerIndex_);
}

void GMCanvas::setActiveLayer(unsigned char index)
{
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
