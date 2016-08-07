#include "canvas.h"

#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QList>
#include <QVector4D>
#include <QUndoCommand>
#include <QMessageBox>

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

     currentMeshHandler()->clearAll();
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
        currentMeshHandler()->importGuiMesh(location);
        //x,y,z = position, w = vertexHandleIdx
        vector<QVector4D> vertices = currentMeshHandler()->vertices();
        for(QVector4D vertex : vertices){
            CanvasItemPoint *item = new CanvasItemPoint();
            QPointF point(vertex.x(), vertex.y());
            item->setPos(point);
            item->setVertexHandleIdx(vertex.w());
            addControlPoint(item);
        }

        vector<QVector4D> edges = currentMeshHandler()->edges();
        for(QVector4D edge : edges){
            //TODO add edge
        }

    }
    else
    {
        currentMeshHandler()->saveGuiMeshOff(location);
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
    currentMeshHandler()->prepareGuiMeshForSubd();
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

void GMCanvas::showMessage(QString message, bool eraseLastVertToAddFace)
{
    if(eraseLastVertToAddFace) vertsToAddFace_.erase(vertsToAddFace_.end()-1);
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
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
        //If collide check if face should be made, else add point
        if(collide)
        {
            CanvasItemPoint *collidePoint = layers_.at(currLayerIndex_)->points.at(collideWithIndex);
            if (vertsToAddFace_.size() == 0)
            {
                //Face start at an already existing vertex
                vertsToAddFace_.push_back(collidePoint);
            }
            else
            {
                if(collidePoint != vertsToAddFace_.at(0))
                {
                    if(currentMeshHandler()->numberOfFaces() != 0)
                    {
                        vertsToAddFace_.push_back(collidePoint);
                    }
                    else
                    {
                        showMessage("First face must be closed. Choose the first vertex to close and make face.");
                        return;
                    }

                    //Check if last point is connected to face, dosn't apply to first face
                    if(currentMeshHandler()->vertexValence(vertsToAddFace_.back()->vertexHandleIdx()) == 0
                            && currentMeshHandler()->numberOfFaces() > 0 )
                    {
                        //Delete the last point, that should not be a part of face.
                        showMessage("Last vertex must be connected to a face", true);
                        return;
                    }
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

                //Check if mesh start and end within same face, to avoid looping condition.
                if(faceInsideFace)
                {

                }


                //Special case if size == 2
                if (vertsToAddFace_.size() == 2) faceInsideFace = true;


                bool resetToAddFace = false;
                //If only one point is in list, it should not be added.
                if(vertsToAddFace_.size() >= 2)
                {
                    bool sucsess = currentMeshHandler()->makeFace(vertesToAddFaceIdx, faceInsideFace);
                    qDebug() << "Made face?" << sucsess;

                    if(sucsess)
                    {
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
                    else
                    {
                        resetToAddFace = true;
                    }
                }
                else
                {
                    resetToAddFace = true;
                }

                //Fresh start if the user does something "illegal"
                if(resetToAddFace)
                {
                    vertsToAddFace_.clear();
                    resetLineStartEnd();
                    return;
                }
            }
        }
        else
        {
            if(currentMeshHandler()->numberOfFaces() > 0 && vertsToAddFace_.size()==0)
            {
                showMessage("First vertex must be connected to face");
                return;
            }

            addControlPoint(itemPoint);

            //The point is new, and should be added to a new face.
            vertsToAddFace_.push_back(itemPoint);
            qDebug() << "added to face";
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

       //Make line between the two points.
       if(lineStartPoint_ != nullptr && lineEndPoint_ != nullptr && lineStartPoint_ != lineEndPoint_)
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
    int vertexHandleIdx = currentMeshHandler()->addVertex(item->pos(), pointColor_);
    item->setVertexHandleIdx(vertexHandleIdx);

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
