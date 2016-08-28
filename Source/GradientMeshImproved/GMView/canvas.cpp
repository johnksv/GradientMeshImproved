    #include "canvas.h"
    #include "openglwidget.h"
    #include <QGraphicsSceneMouseEvent>
    #include <QDebug>
    #include <QGraphicsProxyWidget>
    #include <QList>
    #include <QVector4D>
    #include <QUndoCommand>
    #include <QMessageBox>
    #include "canvaspointconstraint.h"
    #include "canvaspointdiscontinued.h"

    using namespace GMView;

    GMCanvas::GMCanvas(QObject * parent):
        QGraphicsScene(parent)
    {
        meshHandlers_.push_back(new GUILogic::MeshHandler);
        GMOpenGLWidget *openglWidget = new GMOpenGLWidget(this, nullptr);
        opengl_ = addWidget(openglWidget);
        opengl_->setPos(0,0);
        opengl_->setZValue(0);

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
         CanvasItemGroup* layer = currentLayer();
         layer->faces.clear();
         layer->lines.clear();
         layer->points.clear();


         for(QGraphicsItem* item: layer->childItems())
         {
             delete item;
         }


         currentMeshHandler()->clearAll();

         vertsToAddFace_.clear();
         resetLineStartEnd();
         static_cast<GMOpenGLWidget*>(opengl_->widget())->paintGL();
         update();
    }

    void GMCanvas::resetLineStartEnd()
    {
        if(vertsToAddFace_.empty())
        {
            lineStartPoint_ = nullptr;
            lineEndPoint_ = nullptr;
        }
    }

    void GMCanvas::handleFileDialog(QString location, bool import)
    {
        if(import)
        {
            qDebug() << "scne import";
            currentMeshHandler()->importGuiMesh(location);
            //x,y,z = position, w = vertexHandleIdx
            constructGuiFromMeshHandler();

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

            //Allow for rubber band selection
            if(mouseEvent->modifiers() & Qt::ShiftModifier) break;

            if(! (mouseEvent->modifiers() & Qt::ControlModifier))
            {
                clearSelection();
            }

            QGraphicsScene::mousePressEvent(mouseEvent);
            break;
        case drawModeCanvas::lineTool:
            clearSelection();

            if(mouseEvent->modifiers() & Qt::ControlModifier)
            {
                QGraphicsScene::mousePressEvent(mouseEvent);
                break;
            }


            mouseLineTool(mouseEvent);
            mouseEvent->accept();
            break;
        case drawModeCanvas::circleTool:
            clearSelection();

            if(mouseEvent->modifiers() & Qt::ControlModifier)
            {
                QGraphicsScene::mousePressEvent(mouseEvent);
                break;
            }

            mouseCircleTool(mouseEvent);
            mouseEvent->accept();
            break;
        }

    }

    void GMCanvas::setDrawColorVertex(QColor pointColor)
    {
        pointColor_ = pointColor;
    }

    void GMCanvas::setRenderConstraintHandlers(bool value)
    {
        renderConstraintHandlers_ = value;
    }

    void GMCanvas::setRenderVertsEdges(bool renderVertsEdges)
    {
        renderVertsEdges_ = renderVertsEdges;
        for(CanvasItemGroup* itemGroup : layers_)
        {
            itemGroup->setVisible(renderVertsEdges_);
        }
    }

    drawModeCanvas GMCanvas::drawingMode() const
    {
        return drawMode_;
    }

    renderModeCanvas GMCanvas::renderMode() const
    {
        return renderingMode_;
    }

    bool GMCanvas::renderVertsEdges() const
    {
        return renderVertsEdges_;
    }

    bool GMCanvas::renderConstraintHandlers() const
    {
        return renderConstraintHandlers_;
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

    CanvasItemGroup *GMCanvas::currentLayer()
    {
        return layers_.at(currLayerIndex_);
    }

    vector<GUILogic::MeshHandler *> *GMCanvas::meshHandlers()
    {
        return &meshHandlers_;
    }

    GUILogic::MeshHandler *GMCanvas::currentMeshHandler()
    {
		if(multiRes_meshHandlers_.empty()) return meshHandlers_.at(currLayerIndex_);
        return multiRes_meshHandlers_.at(currLayerIndex_);
    }

    vector<GUILogic::MeshHandler *> *GMCanvas::multiResMeshHandlers()
    {
        return &multiRes_meshHandlers_;
    }

    void GMCanvas::setActiveLayer(unsigned char index)
    {
        currentLayer()->QGraphicsItem::setEnabled(false);
        if(index < 0 || index >= layers_.size())
        {
            currLayerIndex_ = 0;
        }
        else
        {
            currLayerIndex_ = index;
        }
        currentLayer()->QGraphicsItem::setEnabled(true);
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

	void GMCanvas::drawOpenGlOnCanvas(bool drawOnCanvas)
	{
        static_cast<GMOpenGLWidget*>(opengl_->widget())->setVisible(drawOnCanvas);
	}

	void GMCanvas::prepareRendering()
	{
        vector<GUILogic::MeshHandler*> tempMeshHandler;

        if(multiRes_meshHandlers_.empty()) tempMeshHandler = meshHandlers_;
        else tempMeshHandler = multiRes_meshHandlers_;

        for (int i = 0; i < tempMeshHandler.size(); ++i) {
            tempMeshHandler.at(i)->prepareGuiMeshForSubd();
        }
        static_cast<GMOpenGLWidget*>(opengl_->widget())->paintGL();
		update();
	}

	void GMView::GMCanvas::multiResFirstStepMesh()
	{
        for(int i = 0; i < meshHandlers()->size(); i++)
        {
            //Execute render.
            meshHandlers_.at(i)->prepareGuiMeshForSubd();

            //Load mesh (that is one step subdivided) into openMesh
            GUILogic::MeshHandler *multiresMesh = meshHandlers_.at(i)->oneStepSubdMesh();

            //Add first step subdivided mesh to containers.
            CanvasItemGroup *guiRepresentation = new CanvasItemGroup("subdivieded mesh");
            multiRes_layers_.push_back(guiRepresentation);
            addItem(guiRepresentation);
            multiRes_meshHandlers_.push_back(multiresMesh);

            //Hide "Orginal layer"
            if(!currentLayer()->isVisible())
            {
                 multiRes_layers_.back()->hide();
            }
			currentLayer()->setVisible(false);
							
			constructGuiFromMeshHandler(true, i);
        }
    }

    void GMCanvas::resetMultiResMesh()
    {
        for(int i = 0; i < multiRes_meshHandlers_.size(); i++)
        {
            delete multiRes_meshHandlers_.at(i);
            delete multiRes_layers_.at(i);
            layers().at(i)->setVisible(false);
        }
        multiRes_meshHandlers_.clear();
        multiRes_layers_.clear();
    }

    void GMCanvas::mouseLineTool(QGraphicsSceneMouseEvent *event)
    {
        bool madeFace = false;
        bool collide = false;
        int collideWithIndex;

        CanvasItemPoint *itemPoint = new CanvasItemPoint(pointColor_);
        itemPoint->setPos(event->scenePos());

        for(int i = 0; i < currentLayer()->points.size();i++)
        {
            if(itemPoint->collidesWithItem(currentLayer()->points.at(i))){
                collide = true;
                collideWithIndex = i;
                i = currentLayer()->points.size();
            }
        }
        if(event->button() == Qt::LeftButton)
        {
            //If collide check if face should be made, else add point
            if(collide)
            {
                CanvasItemPoint *collidePoint = currentLayer()->points.at(collideWithIndex);
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
					else if (currentMeshHandler()->numberOfFaces() != 0)
					{
						//TODO: Discusse; implement good solution for face start and end at same vert
						showMessage("Face can not start and end on same vertex..");
						return;
						
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

                    //Special case if size == 2
                    if (vertsToAddFace_.size() == 2) faceInsideFace = true;

                    //Check if mesh start and end within same face, to avoid one looping condition. Optimaly it should also check all the other points to..
                    if(faceInsideFace)
                    {
                        int idxFront = vertsToAddFace_.front()->vertexHandleIdx();
                        int idxBack = vertsToAddFace_.back()->vertexHandleIdx();

                        //This will result in loop (that is catched later), if the the two verts are added over existing faces
                        bool boundaryVerts= currentMeshHandler()->isBoundaryVertex(idxFront) && currentMeshHandler()->isBoundaryVertex(idxBack);

                        bool sameFace = currentMeshHandler()->vertsOnSameFace(idxFront, idxBack);
                        if(!sameFace && !boundaryVerts)
                        {
                            showMessage("Start and end vertex must be on same face", true);
                            return;
                        }
                    }

                    bool resetToAddFace = false;
                    //If only one point is in list, it should not be added.
                    if(vertsToAddFace_.size() >= 2)
                    {
                        bool sucsess;
                        try{
                            sucsess = currentMeshHandler()->makeFace(vertesToAddFaceIdx, faceInsideFace);
                        }
                        catch(int e)
                        {
                            if (e==-1)
                            {
                                showMessage("Second last point must be inside same face as first. Adjust, and select end vertex again", true);
                                return;
                            }
                        }

                        qDebug() << "Made face?" << sucsess;

                        if(sucsess)
                        {
                            //For debugging purposes
                            CanvasItemFace * face = new CanvasItemFace();
                            currentLayer()->faces.push_back(face);

                            for(int handle : vertesToAddFaceIdx)
                            {
                                for(CanvasItemPoint *point : currentLayer()->points)
                                {
                                    if(point->vertexHandleIdx() == handle)
                                    {
                                        face->addCanvasPoint(point);
                                    }
                                }
                            }
                            currentLayer()->addToGroup(face); //End debugging purposes
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
            }



           if(lineStartPoint_ == nullptr){
               if(collide){
                    lineStartPoint_ = currentLayer()->points.at(collideWithIndex);
               }else{
                   lineStartPoint_ = itemPoint;
               }
           }else{
               if(collide) {
                    lineEndPoint_ = currentLayer()->points.at(collideWithIndex);
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
               int size = currentLayer()->lines.size();
               for(int i = 0; i < size; i++ )
               {
                   if(*(currentLayer()->lines.at(i)) == *line	||
                            ( currentLayer()->lines.at(i)->endPoint() == line->startPoint() &&
                              currentLayer()->lines.at(i)->startPoint() == line->endPoint()))
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
                   currentLayer()->addToGroup(line);

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

    void GMCanvas::mouseCircleTool(QGraphicsSceneMouseEvent *event)
    {
        bool madeFace = false;
        bool collide = false;
        int collideWithIndex;

        CanvasItemPoint *itemPoint = new CanvasItemPoint(pointColor_);
        itemPoint->setPos(event->scenePos());

        for(int i = 0; i < currentLayer()->points.size();i++)
        {
            if(itemPoint->collidesWithItem(currentLayer()->points.at(i))){
                collide = true;
                collideWithIndex = i;
                i = currentLayer()->points.size();
            }
        }
        if(event->button() == Qt::LeftButton)
        {
            //If collide check if face should be made, else add point
            if(collide)
            {
                CanvasItemPoint *collidePoint = currentLayer()->points.at(collideWithIndex);
                if(vertsToAddFace_.size() == 0)
                {
                    //Add point to list
                    vertsToAddFace_.push_back(collidePoint);
                }
                else if (collidePoint != vertsToAddFace_.front())
                {
                    //Add point to list
                    vertsToAddFace_.push_back(collidePoint);
                }
                else
                {
                    //Make Face

                    vector<int> vertesToAddFaceIdx;
                    for (int i = 0; i < vertsToAddFace_.size(); i++)
                    {
                        CanvasItemPoint *itemPoint = vertsToAddFace_.at(i);
                        vertesToAddFaceIdx.push_back(itemPoint->vertexHandleIdx());
                    }

                    if (vertsToAddFace_.size() < 3)
                    {
                        showMessage("Face must consist of 3 vertecies minimum 3 vertecies");
                    }

                    qDebug() << "Made face?" << currentMeshHandler()->addFaceClosed(vertesToAddFaceIdx);

                    //For debugging purposes
                    CanvasItemFace * face = new CanvasItemFace();
                    currentLayer()->faces.push_back(face);

                    for(int handle : vertesToAddFaceIdx)
                    {
                        for(CanvasItemPoint *point : currentLayer()->points)
                        {
                            if(point->vertexHandleIdx() == handle)
                            {
                                face->addCanvasPoint(point);
                            }
                        }
                    }
                    currentLayer()->addToGroup(face); //End debugging purposes
                    madeFace = true;
                    vertsToAddFace_.clear();

                }
            }
            else
            {
                addControlPoint(itemPoint);

                //The point is new, and should be added to a new face.
                vertsToAddFace_.push_back(itemPoint);
            }



           if(lineStartPoint_ == nullptr){
               if(collide){
                    lineStartPoint_ = currentLayer()->points.at(collideWithIndex);
               }else{
                   lineStartPoint_ = itemPoint;
               }
           }else{
               if(collide) {
                    lineEndPoint_ = currentLayer()->points.at(collideWithIndex);
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
               int size = currentLayer()->lines.size();
               for(int i = 0; i < size; i++ )
               {
                   if(*(currentLayer()->lines.at(i)) == *line	||
                            ( currentLayer()->lines.at(i)->endPoint() == line->startPoint() &&
                              currentLayer()->lines.at(i)->startPoint() == line->endPoint()))
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
                   currentLayer()->addToGroup(line);

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

        currentLayer()->addToGroup(item);
        update(item->boundingRect());
    }

    void GMCanvas::showMessage(QString message, bool eraseLastVertToAddFace)
    {
        if(eraseLastVertToAddFace) vertsToAddFace_.pop_back();
        QMessageBox msgBox;
        msgBox.setText(message);
        msgBox.exec();
    }

    void GMCanvas::constructGuiFromMeshHandler(bool fromMultiRes, int indexOfMultiResLayer)
    {
        GUILogic::MeshHandler* meshhandler;
        CanvasItemGroup *layer;
        if(fromMultiRes)
        {
            meshhandler = multiRes_meshHandlers_.at(indexOfMultiResLayer);
            layer = multiRes_layers_.at(indexOfMultiResLayer);
        }
        else
        {
            meshhandler = currentMeshHandler();
            layer = currentLayer();
        }
        vector<QVector4D> vertices = meshhandler->vertices();
        for(QVector4D vertex : vertices){
            CanvasItemPoint *item = new CanvasItemPoint();
            QPointF point(vertex.x(), vertex.y());
            item->setPos(point);
            item->setVertexHandleIdx(vertex.w());

            layer->addToGroup(item);
        }

        int startIdx,endIdx;
        vector<QVector4D> edges = meshhandler->edges();
        for(QVector4D edge : edges)
		{
            //Returns  startVert.idx(),endVert.idx(), e_it->idx(), 0
            startIdx = edge.x();
            endIdx = edge.y();
            CanvasItemPoint* startPoint = nullptr;
            CanvasItemPoint* endPoint = nullptr;

            for(CanvasItemPoint* point : layer->points)
            {
                int pointIdx = point->vertexHandleIdx();

                if(pointIdx == startIdx)
                {
                    startPoint = point;
                }

                if(pointIdx == endIdx)
                {
                    endPoint = point;
                }

                if(startPoint != nullptr && endPoint != nullptr) break;
            }

            if(startPoint == nullptr || endPoint == nullptr)
            {
                qDebug() << "Oh no..." << endIdx;
            }
            else
            {
                CanvasItemLine *line = new CanvasItemLine(startPoint, endPoint);
                layer->addToGroup(line);

                //TODO: Get constraints from import file.
                CanvasPointConstraint *startConstraint = new CanvasPointConstraint(startPoint, line);
                startConstraint->setPos(QPointF(line->line().dx()*0.2, line->line().dy()*0.2));

                CanvasPointConstraint *endConstraint = new CanvasPointConstraint(endPoint, line);
                endConstraint->setPos(QPointF(line->line().dx()*-0.2, line->line().dy()*-0.2));

            }
        }
    }
