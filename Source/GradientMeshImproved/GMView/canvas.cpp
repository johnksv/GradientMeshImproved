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
    #include <QToolTip>


    using namespace GMView;

GMCanvas::GMCanvas(QObject * parent):
        QGraphicsScene(parent)
    {
        initGMCanvas();

        //TODO: Change sceneRect(?)
        setSceneRect(itemsBoundingRect());

        //Faster?? TODO: Test if faster then BspTreeIndex (standard)
        setItemIndexMethod(QGraphicsScene::NoIndex);
    }

    GMCanvas::~GMCanvas()
    {
        for(GUILogic::MeshHandler *mesh : meshHandlers_)
        {
            delete mesh;
        }
    }
    void GMCanvas::initGMCanvas()
    {
        meshHandlers_.push_back(new GUILogic::MeshHandler);
        GMOpenGLWidget *openglWidget = new GMOpenGLWidget(this, nullptr);
        opengl_ = addWidget(openglWidget);
        opengl_->setPos(0,0);
        opengl_->setZValue(-1);

        CanvasItemGroup *layer = new CanvasItemGroup("Layer 1");
        layers_.push_back(layer);
        addItem(layer);
    }

    void GMCanvas::clear()
    {
        QGraphicsScene::clear();
        for (int i = 0; i < meshHandlers_.size(); ++i)
        {
             delete meshHandlers_.at(i);
        }
        layers_.erase(layers_.begin(),layers_.end());
        meshHandlers_.erase(meshHandlers_.begin(), meshHandlers_.end());


        if(!multiRes_meshHandlers_.empty())
        {
            for (int i = 0; i < multiRes_meshHandlers_.size(); ++i) {
                delete multiRes_meshHandlers_.at(i);
                delete multiRes_layers_.at(i);
            }
            multiRes_meshHandlers_.clear();
            multiRes_layers_.clear();
        }

        initGMCanvas();
        currLayerIndex_ = 0;

        static_cast<GMOpenGLWidget*>(opengl_->widget())->paintGL();
        update();
    }

    void GMCanvas::clearAllCurrLayer(bool clearMeshHandler)
    {
        CanvasItemGroup* layer = currentLayer();
        layer->clear();

        if(clearMeshHandler) currentMeshHandler()->clearAll();

        vertsToAddFace_.clear();
        resetLineStartEnd();
        static_cast<GMOpenGLWidget*>(opengl_->widget())->paintGL();
        update();
    }

    void GMCanvas::resetToBeFaceVector()
    {
        for (int i = 0; i < vertsToAddFace_.size(); ++i) {
            CanvasItemPoint* point = vertsToAddFace_.at(i);
            int idx = point->vertexHandleIdx();
            if(currentMeshHandler()->vertexValence(idx) < 1)
            {
                foreach (QGraphicsItem* item, currentLayer()->lines().childItems())
                {
                    CanvasItemLine* edge = static_cast<CanvasItemLine*> (item);
                    if(point == edge->startPoint() || point == edge->endPoint())
                    {
                        delete edge;
                        break;
                    }
                }
                currentMeshHandler()->removeVertex(idx);


                delete vertsToAddFace_.at(i);
            }
            else if(i==0 && vertsToAddFace_.size() > 1)
            {
                foreach (QGraphicsItem* pointChild, point->childItems())
                {
                    CanvasPointConstraint* pointConstraint = dynamic_cast<CanvasPointConstraint*>(pointChild);
                    if(pointConstraint != nullptr)
                    {
                        if(pointConstraint->edge()->endPoint() == vertsToAddFace_.at(1)){
                            delete pointConstraint;
                        }
                    }
                }
            }
        }
        vertsToAddFace_.clear();
        resetLineStartEnd();

        currentMeshHandler()->garbageCollectOpenMesh();
    }

    void GMCanvas::resetLineStartEnd()
    {
        if(vertsToAddFace_.empty())
        {
            lineStartPoint_ = nullptr;
            lineEndPoint_ = nullptr;
        }
        currentLayer()->resetPointsHighlighted();
    }

    void GMCanvas::handleFileDialog(QString location, bool import)
    {
        if(import)
        {
            qDebug() << "scne import";
			if (currentMeshHandler()->numberOfFaces() > 0)
			{
				QString message("Import of file will clean current mesh. Proceed?");
				QMessageBox::StandardButton response = QMessageBox::question(nullptr, "Multi resh mesh", message);
				if (response == QMessageBox::Yes)
				{
					importFile(location);
				}
			}
			else
			{
				importFile(location);
			}
        }
        else
        {
            currentMeshHandler()->prepareGuiMeshForSubd(true, location);
        }
    }

	void GMCanvas::importFile(QString location)
	{
		clearAllCurrLayer();
        currentMeshHandler()->importGuiMesh(location, renderAutoUpdate_);
        constructGuiFromMeshHandler();
    }

    void GMCanvas::handleImageFileDialog(QString location, bool import)
    {
        delete imageItem_;
        if(import)
        {
            imageItem_ = addPixmap(QPixmap(location));
            imageItem_->setZValue(-1);
        }
		else
		{
            imageItem_ = nullptr;
        }
    }

    QGraphicsPixmapItem *GMCanvas::imageItem()
    {
        return imageItem_;
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

        case drawModeCanvas::collapseEdge:
            mouseCollapseEdge(mouseEvent);
            break;

        case drawModeCanvas::insertVert:
            mouseInsertVertOnEdge(mouseEvent);
            mouseEvent->accept();
            break;
        }
    }

    void GMCanvas::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
    {
         if(drawMode_ !=drawModeCanvas::move){
            event->accept();
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
        for(CanvasItemGroup* itemGroup : layers_)
        {
            itemGroup->setVisible(renderVertsEdges);
        }
    }

    void GMCanvas::setRenderAuto(bool renderAutoUpdate)
    {
        renderAutoUpdate_ = renderAutoUpdate;
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


    void GMCanvas::setRenderingMode(renderModeCanvas mode)
    {
        renderingMode_ = mode;
    }

    void GMCanvas::setDrawingMode(drawModeCanvas drawMode)
    {
        resetLineStartEnd();
        this->drawMode_ = drawMode;
    }

    vector<CanvasItemGroup *> GMCanvas::layers()
    {
        return layers_;
    }

    CanvasItemGroup *GMCanvas::currentLayer()
    {
        if(multiRes_meshHandlers_.empty()) return layers_.at(currLayerIndex_);
        return multiRes_layers_.at(currLayerIndex_);
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
        //Disable other layers for editing when active layer is changed.
        if(currLayerIndex_ < layers_.size()) currentLayer()->QGraphicsItem::setEnabled(false);
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
                //QT handels deletion of each object.
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
        updateVertexConstraints();
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
        for(int i = 0; i < meshHandlers_.size(); i++)
        {
            //Execute render.
            meshHandlers_.at(i)->prepareGuiMeshForSubd();

            //Load mesh (which is one step subdivided) into openMesh
            GUILogic::MeshHandler *multiresMesh = meshHandlers_.at(i)->oneStepSubdMesh();

            //Add first step subdivided mesh to containers.
            CanvasItemGroup *guiRepresentation = new CanvasItemGroup("subdivieded mesh");
            multiRes_layers_.push_back(guiRepresentation);
            addItem(guiRepresentation);
            multiRes_meshHandlers_.push_back(multiresMesh);

            //Hide "Orginal layer"
            if(!layers_.at(currLayerIndex_)->isVisible())
            {
                 multiRes_layers_.back()->hide();
            }
			layers_.at(currLayerIndex_)->setVisible(false);
							
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
        vector<vector<QVector4D>> vertices = meshhandler->vertices();
        for (int i = 0; i < vertices.size(); ++i) {
			vector<QVector4D> vertData = vertices.at(i);

			QPointF point(vertData.at(0).x(), vertData.at(0).y());
			QColor color(vertData.at(1).x(), vertData.at(1).y(), vertData.at(1).z());
            CanvasItemPoint *item = new CanvasItemPoint(color);
            item->setVertexHandleIdx(vertData.at(0).w());
            item->setPos(point);

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
            QList<QGraphicsItem*> items = layer->points().childItems();
            for (int i = 0; i < items.size(); ++i) {
                CanvasItemPoint* point = static_cast<CanvasItemPoint*> (items.at(i));
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

        vector<vector<int>> faces = meshhandler->facesIdx();
		for (int i = 0; i < faces.size(); i++)
		{
            addEdgesToCanvasFace(faces.at(i), i);
		}
    }

    void GMCanvas::updateVertexConstraints()
    {
        //TODO: Improve effeciency
        QList<QGraphicsItem*> items = currentLayer()->points().childItems();
        for (int i = 0; i < items.size(); ++i) {
            CanvasItemPoint* point = static_cast<CanvasItemPoint*> (items.at(i));

            QList<QGraphicsItem *> children = point->childItems();
            for (int i = 0; i < children.size(); ++i) {
                CanvasPointConstraint* constraint = dynamic_cast<CanvasPointConstraint*> (children.at(i));
                if(constraint)
                {
                        constraint->updatePosInOpenMesh();
                }
            }
        }
    }

    void GMCanvas::mouseLineTool(QGraphicsSceneMouseEvent *event)
    {
        bool madeFace = false;

        CanvasItemPoint *itemPoint = new CanvasItemPoint(pointColor_);
        itemPoint->setPos(event->scenePos());

        CanvasItemPoint* collidePoint = dynamic_cast<CanvasItemPoint*>(findCollideWithPoint(itemPoint));

        bool collide = collidePoint != nullptr ? true : false;

        if(event->button() == Qt::LeftButton)
        {
            vector<int> vertsToAddFaceIdx;

            //If collide check if face should be made, else add point
            if(collide)
            {
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

                    //If the face to be should be added inside an already existing face
                    bool faceInsideFace = false;
                    vector<CanvasItemFace*> faces = currentLayer()->faces();

                    //push IDX to the vector that is passed to MeshHandler.
                    for (int i = 0; i < vertsToAddFace_.size(); i++)
                    {
                        CanvasItemPoint *itemPoint = vertsToAddFace_.at(i);
                        vertsToAddFaceIdx.push_back(itemPoint->vertexHandleIdx());

                        //Check if face is inside face.
                        //No need to check last or first item,.
                        if (i != 0 && i < vertsToAddFace_.size() - 1)
                        {
                            for (int i = 0; i < faces.size(); i++)
                            {
                                if(faces.at(i)->contains(itemPoint->pos()))
                                {
                                    faceInsideFace = true;
                                    break;
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
                            sucsess = currentMeshHandler()->makeFace(vertsToAddFaceIdx, faceInsideFace);
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
                            autoRenderOnMeshChanged();
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

           addItemLine(itemPoint, collidePoint, collide);

           //Startpoint should be reset if a face was made.
           if(madeFace)
           {
               lineStartPoint_ = nullptr;

               //Add edges to canvas face after the last edge/ItemLine is added
               addEdgesToCanvasFace(vertsToAddFaceIdx, currentMeshHandler()->numberOfFaces()-1);
           }

           //Should not delete ItemPoint from heap.
           return;
        }
        //No use for item. Delete it from heap.
        delete itemPoint;
    }

    void GMCanvas::addItemLine(CanvasItemPoint* itemPoint, CanvasItemPoint* collidePoint, bool collide){
        if(lineStartPoint_ == nullptr){
            if(collide){
                lineStartPoint_ = collidePoint;
            }else{
                lineStartPoint_ = itemPoint;
            }
        }else{
            if(collide) {
                lineEndPoint_ = collidePoint;
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
            QList<QGraphicsItem*> items = currentLayer()->lines().childItems();
            for(int i = 0; i < items.size(); i++ )
            {
                CanvasItemLine *itemLine  = static_cast<CanvasItemLine*>(items.at(i));
                if(*(itemLine) == *line	||
                        ( itemLine->endPoint() == line->startPoint() &&
                        itemLine->startPoint() == line->endPoint()))
                {
                    exists = true;
                    break;
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
    }

    void GMCanvas::mouseCircleTool(QGraphicsSceneMouseEvent *event)
    {
        bool madeFace = false;

        CanvasItemPoint *itemPoint = new CanvasItemPoint(pointColor_);
        itemPoint->setPos(event->scenePos());

        CanvasItemPoint* collidePoint = dynamic_cast<CanvasItemPoint*>(findCollideWithPoint(itemPoint));
        bool collide = collidePoint != nullptr ? true : false;

        if(event->button() == Qt::LeftButton)
        {
            vector<int> vertsToAddFaceIdx;
            //If collide check if face should be made, else add point
            if(collide)
            {
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
                    for (int i = 0; i < vertsToAddFace_.size(); i++)
                    {
                        CanvasItemPoint *itemPoint = vertsToAddFace_.at(i);
                        vertsToAddFaceIdx.push_back(itemPoint->vertexHandleIdx());
                    }

                    if (vertsToAddFace_.size() < 3)
                    {
                        showMessage("Face must consist of 3 vertecies minimum 3 vertecies");
						return;
                    }

                    qDebug() << "Made face?" << currentMeshHandler()->addFaceClosed(vertsToAddFaceIdx);
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

           addItemLine(itemPoint, collidePoint, collide);
           //Startpoint should be reset if a face was made.
           if(madeFace){
               lineStartPoint_ = nullptr;
               addEdgesToCanvasFace(vertsToAddFaceIdx, currentMeshHandler()->numberOfFaces()-1);
           }

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

    QGraphicsItem* GMCanvas::findCollideWithPoint(CanvasItemPoint *itemPoint)
    {

        foreach (QGraphicsItem* collidepoint, currentLayer()->points().childItems()) {
            if(itemPoint->collidesWithItem(collidepoint)){
                return collidepoint;
            }
        }
        return nullptr;
    }

    void GMCanvas::autoRenderOnMeshChanged()
    {
        if(renderAutoUpdate_)
        {
            prepareRendering();
            emit GUIMeshChanged();
        }
    }
    void GMCanvas::mouseCollapseEdge(QGraphicsSceneMouseEvent *mouseEvent)
    {
        CanvasItemPoint *itemPoint = new CanvasItemPoint(pointColor_);
        itemPoint->setPos(mouseEvent->scenePos());

        CanvasItemPoint* collidePoint = dynamic_cast<CanvasItemPoint*>(findCollideWithPoint(itemPoint));
        bool collide = collidePoint != nullptr ? true : false;


        if(lineStartPoint_ == nullptr){
            if(collide)
            {
                lineStartPoint_ = collidePoint;
                QList<QGraphicsItem*> edges = currentLayer()->lines().childItems();
                for (int i = 0; i < edges.size(); ++i)
                {
                    CanvasItemLine* edge = static_cast<CanvasItemLine*>(edges.at(i));
                    if(edge->startPoint() == lineStartPoint_ )
                    {
                        edge->endPoint()->setHighlighted(true);
                    }
                    else if(edge->endPoint()== lineStartPoint_)
                    {
                        edge->startPoint()->setHighlighted(true);
                    }
                }

            }
        }else{
            if(collide)
            {
                lineEndPoint_ = collidePoint;
                if(! lineEndPoint_ ->isHighlighted())
                {
                    showMessage("Illegal endPoint. Must be direct neighbour to the first point-");
                    resetLineStartEnd();
                }
            }
        }

        //Make line between the two points.
        if(lineStartPoint_ != nullptr && lineEndPoint_ != nullptr && lineStartPoint_ != lineEndPoint_)
        {

            bool collapse = currentMeshHandler()->collapseEdge(lineStartPoint_->vertexHandleIdx(),lineEndPoint_->vertexHandleIdx());
            if(collapse)
            {
                clearAllCurrLayer(false);
                constructGuiFromMeshHandler();
            }
            else
            {
                showMessage("Collapse not possible. Will make mesh inconsistent!");
            }
        }
    }

    void GMCanvas::mouseInsertVertOnEdge(QGraphicsSceneMouseEvent *event)
    {
        QPointF position = event->scenePos();
        CanvasItemLine *edge = dynamic_cast<CanvasItemLine*>(itemAt(position, QTransform()));
        foreach (QGraphicsItem* item, items(position)) {
            edge = dynamic_cast<CanvasItemLine*>(item);
            if(edge !=nullptr) break;
        }

        if(edge != nullptr)
        {
            int startIdx = edge->startPoint()->vertexHandleIdx();
            int endIdx = edge->endPoint()->vertexHandleIdx();
            currentMeshHandler()->insertVertexOnEdge(startIdx, endIdx, position, pointColor_);

            //TODO: Reimplement. that is: in GUI: make new edges, update face
            //For now it's easiest to construct the mesh from the meshhandler
            clearAllCurrLayer(false);
            constructGuiFromMeshHandler();
        }
        else
        {
            //TODO: Give feedback to user that edges was not clicked(?)
        }

    }

    void GMCanvas::addEdgesToCanvasFace(const vector<int> &vertsToAddFaceIdx, int faceIdx)
    {
        CanvasItemFace * face = new CanvasItemFace(currentLayer(), faceIdx);
        currentLayer()->addToFacesVector(face);

        vector<CanvasItemLine *> canvasEdges;

        canvasEdges.push_back(edgeBetweenPoints(vertsToAddFaceIdx.front(), vertsToAddFaceIdx.back()));
        for (int i = 1; i < vertsToAddFaceIdx.size(); ++i)
        {
            canvasEdges.push_back(edgeBetweenPoints(vertsToAddFaceIdx.at(i-1), vertsToAddFaceIdx.at(i)));
        }

        face->addCanvasEdge(canvasEdges);
    }

    CanvasItemLine *GMCanvas::edgeBetweenPoints(const int &vertIdPoint1,const int &vertIdPoint2)
    {
        QList<QGraphicsItem*> edges = currentLayer()->lines().childItems();
        for (int i = 0; i < edges.size(); ++i)
        {
            CanvasItemLine* edge = static_cast<CanvasItemLine*> (edges.at(i));


            if(     (edge->startPoint()->vertexHandleIdx() == vertIdPoint1
                    && edge->endPoint()->vertexHandleIdx() == vertIdPoint2 )
                ||
                    (edge->endPoint()->vertexHandleIdx() == vertIdPoint1
                    && edge->startPoint()->vertexHandleIdx() == vertIdPoint2 ))
            {
                    return edge;

            }
        }
        return nullptr;

    }
