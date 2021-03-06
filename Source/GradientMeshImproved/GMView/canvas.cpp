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
#include <QToolTip>
#include <QGraphicsview>
#include <QRect>
#include "undoCommands/guichange.h"
#include <QImage>
#include <QPushButton>
#include "mainwindow.h"
#include <algorithm>

using namespace GMView;
bool GMView::drawCanvasItemFaces = false;

GMCanvas::GMCanvas(QObject * parent):
    QGraphicsScene(parent)
{
    setSceneRect(-5000,-5000,10000,10000);
    initGMCanvas();
}

GMCanvas::~GMCanvas()
{
    for (int i = 0; i < layersAndMeshHandlers_.size(); ++i) {
        delete layersAndMeshHandlers_.at(i).second;
    }
}

void GMCanvas::initGMCanvas()
{
    GMOpenGLWidget *openglWidget = new GMOpenGLWidget(this, nullptr);
    opengl_ = addWidget(openglWidget);
    opengl_->setPos(0,0);
    openglWidget->setRenderMode(renderModeOpenGL::viewportRender);
    opengl_->setZValue(-1);
    opengl_->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    CanvasItemGroup *layer = new CanvasItemGroup("Layer 1");

    layersAndMeshHandlers_.push_back(std::make_pair(layer, new GUILogic::MeshHandler));
    addItem(layer);
}

void GMCanvas::clear()
{
    QGraphicsScene::clear();
    for (int i = 0; i < layersAndMeshHandlers_.size(); ++i)
    {
         delete layersAndMeshHandlers_.at(i).second;
    }
    layersAndMeshHandlers_.erase(layersAndMeshHandlers_.begin(),layersAndMeshHandlers_.end());


    if(!multiResLayersAndMeshHandlers_.empty())
    {
        for (int i = 0; i < multiResLayersAndMeshHandlers_.size(); ++i) {
            auto pair = multiResLayersAndMeshHandlers_.at(i);
            delete pair.first;
            delete pair.second;
        }
        multiResLayersAndMeshHandlers_.clear();
    }

    initGMCanvas();
    currLayerIndex_ = 0;

    update();
    static_cast<GMOpenGLWidget*>(opengl_->widget())->update();
}

void GMCanvas::clearAllCurrLayer(bool clearMeshHandler)
{
    CanvasItemGroup* layer = currentLayer();
    layer->clear();

    if(clearMeshHandler) currentMeshHandler()->clearAll();

    vertsToAddFace_.clear();
    resetLineStartEnd();
    update();
    static_cast<GMOpenGLWidget*>(opengl_->widget())->update();
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
            currentMeshHandler()->deleteVertex(idx);


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

void GMCanvas::saveViewportToPNG(const QString &location)
{
    updateVertexConstraints();
    clearSelection();
    QImage image(itemsBoundingRect().size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    opengl_->widget()->render(&painter);
    image.save(location);
}

void GMCanvas::saveLayerToOFF(const QString &location)
{
    updateVertexConstraints();
    currentMeshHandler()->prepareMeshForSubd(true, location);
}

void GMCanvas::saveAllLayersToOFF(const QString &location)
{
    updateVertexConstraints();
    for(int i = 0; i < layersAndMeshHandlers_.size(); i++){
        QString loc{location};
        loc.insert(loc.size()-4,QString::number(i));
        layersAndMeshHandlers_.at(i).second->prepareMeshForSubd(true, loc);
    }
}

bool GMCanvas::importFile(const QString &location, const bool firstOfMultipleFiles)
{
    QString message;
    if(firstOfMultipleFiles) {
        message = "It is already control points on this layer. How do you want to insert the first file?";
    }else{
        message = "It is already control points on this layer. How do you want to insert the file?";
    }
    if (currentMeshHandler()->numberOfFaces() > 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Import file");
        msgBox.setInformativeText(message);
        QPushButton *clear = msgBox.addButton("Clear layer", QMessageBox::YesRole);
        QPushButton *asLayer = msgBox.addButton("As new layer", QMessageBox::NoRole);
        QPushButton *none = msgBox.addButton("Cancel import", QMessageBox::RejectRole);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.exec();

        if (msgBox.clickedButton() == clear){
            importFileClean(location);
        }else if(msgBox.clickedButton() == asLayer){
            importFileLayer(location);
        }else{
            delete clear, asLayer, none;
            return false;
        }
        delete clear, asLayer, none;
    }
    else
    {
        importFileClean(location);
    }
    return true;
}



void GMCanvas::importFileClean(QString location)
{
    clearAllCurrLayer();
    currentMeshHandler()->importMesh(location, renderAutoUpdate_);
    constructGuiFromMeshHandler();
    views().first()->fitInView(itemsBoundingRect(),Qt::KeepAspectRatio);
}

void GMCanvas::importFileLayer(QString location)
{
    addLayer("Layer " + QString::number(layersAndMeshHandlers_.size() + 1));
    setActiveLayer(layersAndMeshHandlers_.size()-1);
    importFileClean(location);
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

void GMCanvas::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(drawMode_ == drawModeCanvas::rectangleTool)
    {
        if(lineStartPoint_ != nullptr)
        {
            if(rectItem_ != nullptr)
            {
                rectItem_->setRect(QRectF(lineStartPoint_->pos(),mouseEvent->scenePos()));
            }
            else
            {
                rectItem_ = addRect(QRectF(lineStartPoint_->pos(),mouseEvent->scenePos()),QPen(QColor(0,0,255,150)), QBrush(QColor(0,0,255,150)));
            }
        }
    }
    resizeOpenGLWidget();
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void GMCanvas::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    switch(drawMode_){
    case drawModeCanvas::move:

        //Move items
        if(dynamic_cast<CanvasItemPoint *> (itemAt(mouseEvent->scenePos(),QTransform())) == nullptr
                && dynamic_cast<CanvasPointConstraint *> (itemAt(mouseEvent->scenePos(),QTransform())) == nullptr)
        {
            break;
        }

        if(! (mouseEvent->modifiers() & Qt::ControlModifier) && mouseEvent->button() != Qt::RightButton)
        {
            clearSelection();
        }

        //Allow for rubber band selection
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
    case drawModeCanvas::collapseEdge:
        mouseCollapseEdge(mouseEvent);
        break;

    case drawModeCanvas::insertVert:
        mouseInsertVertOnEdge(mouseEvent);
        mouseEvent->accept();
        break;
    case drawModeCanvas::meshToolInsertion:
        mouseMeshToolInsertion(mouseEvent);
        break;
    case drawModeCanvas::rectangleTool:
        mouseRectangleTool(mouseEvent);
        break;
    }
}


void GMCanvas::setDrawColorVertex(QColor pointColor)
{
    pointColor_ = pointColor;
    QList<QGraphicsItem *> items = selectedItems();
    for(int i = 0; i < items.size(); ++i)
    {
        CanvasItemPoint* point = dynamic_cast<CanvasItemPoint*> (items.at(i));
        if(point != nullptr)
        {
            point->setColor(pointColor_, false);
        }
    }
    if(items.size() > 0) autoRenderOnMeshChanged();
}

void GMCanvas::setRenderConstraintHandlers(bool value)
{
    renderConstraintHandlers_ = value;
}

void GMCanvas::setRenderVertsEdges(bool renderVertsEdges)
{
    for (int i = 0; i < layersAndMeshHandlers_.size(); ++i) {
        CanvasItemGroup *itemGroup = layersAndMeshHandlers_.at(i).first;
        itemGroup->setVisible(renderVertsEdges);
    }
}

void GMCanvas::setRenderAuto(bool renderAutoUpdate)
{
    renderAutoUpdate_ = renderAutoUpdate;
}

void GMCanvas::setRenderFace(bool renderFace)
{
    GMView::drawCanvasItemFaces = renderFace;
}

drawModeCanvas GMCanvas::drawingMode() const
{
    return drawMode_;
}

bool GMCanvas::renderConstraintHandlers() const
{
    return renderConstraintHandlers_;
}

void GMCanvas::setLayerModel(QStandardItemModel *model)
{
    layerModel_ = model;
    for (int i = 0; i < layersAndMeshHandlers_.size(); ++i) {
        CanvasItemGroup *item = layersAndMeshHandlers_.at(i).first;
        layerModel_->appendRow(item);
    }
}


void GMCanvas::setDrawingMode(drawModeCanvas drawMode)
{
    resetLineStartEnd();
    this->drawMode_ = drawMode;
}

void GMCanvas::setActiveLayerBack()
{
    setActiveLayer(layersAndMeshHandlers_.size()-1);
}

void GMCanvas::setActiveLayer(unsigned char index)
{
    //Disable other layers for editing when active layer is changed.
    for(int i = 0; i < layersAndMeshHandlers_.size(); i++){
        layersAndMeshHandlers_.at(i).first->QGraphicsItem::setEnabled(false);
    }
    for(int i = 0; i < multiResLayersAndMeshHandlers_.size(); i++){
        multiResLayersAndMeshHandlers_.at(i).first->QGraphicsItem::setEnabled(false);
    }

    currLayerIndex_ = index;

    currentLayer()->QGraphicsItem::setEnabled(true);
}

vector<CanvasItemGroup *> GMCanvas::layers()
{
    vector<CanvasItemGroup *> groups;
    for(auto &item : layersAndMeshHandlers_){
        groups.push_back(item.first);
    }
    return groups;
}

CanvasItemGroup *GMCanvas::currentLayer()
{
    if(multiResLayersAndMeshHandlers_.empty()) return layersAndMeshHandlers_.at(currLayerIndex_).first;
    return multiResLayersAndMeshHandlers_.at(currLayerIndex_).first;
}

vector<GUILogic::MeshHandler *> GMCanvas::meshHandlers()
{
    vector<GUILogic::MeshHandler *> handlers;
    for (int i = 0; i < layersAndMeshHandlers_.size(); ++i) {
        handlers.push_back(layersAndMeshHandlers_.at(i).second);
    }
    return handlers;
}

GUILogic::MeshHandler *GMCanvas::currentMeshHandler()
{
    if(multiResLayersAndMeshHandlers_.empty()) return layersAndMeshHandlers_.at(currLayerIndex_).second;
    return multiResLayersAndMeshHandlers_.at(currLayerIndex_).second;
}

vector<GUILogic::MeshHandler *> GMCanvas::multiResMeshHandlers()
{
    vector<GUILogic::MeshHandler *> handlers;
    for (int i = 0; i < layersAndMeshHandlers_.size(); ++i) {
        handlers.push_back(layersAndMeshHandlers_.at(i).second);
    }
    return handlers;
}

void GMCanvas::addLayer()
{
    addLayer(QString("Layer " + QString::number(layersAndMeshHandlers_.size() + 1)));
}

void GMCanvas::addLayer(QString name)
{
    CanvasItemGroup * layer = new CanvasItemGroup(name);
    addItem(layer);
    layerModel_->appendRow(layer);

    layersAndMeshHandlers_.push_back(std::make_pair(layer,new GUILogic::MeshHandler));
}

void GMCanvas::deleteLayer(int index)
{
    //If the user click the delete button multiple times after each other
    //this check needs to be done
    if(layersAndMeshHandlers_.size() > 1 && index < layersAndMeshHandlers_.size())
    {
        CanvasItemGroup *group = layersAndMeshHandlers_.at(index).first;
        QList<QGraphicsItem*> children = group->childItems();
        for(int i = 0; i < children.size(); i++)
        {
            //QT handels deletion of each object.
            removeItem(children.at(i));
        }
        delete layersAndMeshHandlers_.at(index).second;
        layersAndMeshHandlers_.erase(layersAndMeshHandlers_.begin()+index);
        removeItem(group);

        setActiveLayer(layersAndMeshHandlers_.size()-1);
        layerModel_->removeRow(index);
    }
}

void GMCanvas::moveLayerUp(int indexToMove)
{
    auto iter = layersAndMeshHandlers_.begin();
    iter_swap(iter+indexToMove,iter+indexToMove-1);
    auto row = layerModel_->takeRow(indexToMove);
    layerModel_->insertRow(indexToMove-1, row);
}

void GMCanvas::moveLayerDown(int indexToMove)
{
    auto iter = layersAndMeshHandlers_.begin();
    iter_swap(iter+indexToMove,iter+indexToMove+1);
    auto row = layerModel_->takeRow(indexToMove);
    layerModel_->insertRow(indexToMove+1, row);
}

void GMCanvas::toogleLayerVisibility(int index)
{
    CanvasItemGroup *selectedLayer = multiResLayersAndMeshHandlers_.empty() ? layersAndMeshHandlers_.at(index).first : multiResLayersAndMeshHandlers_.at(index).first;

    if(selectedLayer->isVisible())
    {
        selectedLayer->hide();
        //meshHandlers()->at(index)->setDraw(false);
    }
    else
    {
        selectedLayer->show();
        //meshHandlers()->at(index)->setDraw(true);
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

    if(multiResLayersAndMeshHandlers_.empty())
    {
        for (int i = 0; i < layersAndMeshHandlers_.size(); ++i) {
            tempMeshHandler.push_back(layersAndMeshHandlers_.at(i).second);
        }
    }
    else {
        for (int i = 0; i < multiResLayersAndMeshHandlers_.size(); ++i) {
            tempMeshHandler.push_back(multiResLayersAndMeshHandlers_.at(i).second);
        }
    }

    for (int i = 0; i < tempMeshHandler.size(); ++i) {
        tempMeshHandler.at(i)->prepareMeshForSubd();
    }
    update();
    static_cast<GMOpenGLWidget*>(opengl_->widget())->update();
}

void GMView::GMCanvas::multiResFirstStepMesh()
{
    //Make sure the mesh is up to date (unnecessary if "auto render" is turned on)
    prepareRendering();
    for(int i = 0; i < layersAndMeshHandlers_.size(); i++)
    {
        //Execute render.
        layersAndMeshHandlers_.at(i).second->prepareMeshForSubd();

        //Load mesh (which is x step subdivided) into openMesh
        GUILogic::MeshHandler *multiresMesh = layersAndMeshHandlers_.at(i).second->meshXStepSubdivided();

        //Add first step subdivided mesh to containers.
        CanvasItemGroup *guiRepresentation = new CanvasItemGroup("subdivieded mesh");
        addItem(guiRepresentation);

        auto pair = std::make_pair(guiRepresentation, multiresMesh);
        multiResLayersAndMeshHandlers_.push_back(pair);

        //Hide "Orginal layer"
        if(!layersAndMeshHandlers_.at(currLayerIndex_).first->isVisible())
        {
             multiResLayersAndMeshHandlers_.back().first->hide();
        }
        layersAndMeshHandlers_.at(currLayerIndex_).first->setVisible(false);

        constructGuiFromMeshHandler(true, i);
    }
}

void GMCanvas::resetMultiResMesh()
{
    for(int i = 0; i < multiResLayersAndMeshHandlers_.size(); i++)
    {
        auto pair = multiResLayersAndMeshHandlers_.at(i);
        delete pair.first;
        delete pair.second;
        layersAndMeshHandlers_.at(i).first->setVisible(false);
    }
    multiResLayersAndMeshHandlers_.clear();
}

void GMCanvas::constructGuiFromMeshHandler(bool fromMultiRes, int indexOfMultiResLayer)
{
    GUILogic::MeshHandler* meshhandler;
    CanvasItemGroup *layer;
    if(fromMultiRes)
    {
        meshhandler = multiResLayersAndMeshHandlers_.at(indexOfMultiResLayer).second;
        layer = multiResLayersAndMeshHandlers_.at(indexOfMultiResLayer).first;
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

	//Returns  startVert.idx(),endVert.idx(), e_it->idx(), 0
    vector<QVector4D> edges = meshhandler->edges();
	for (int i = 0; i < edges.size(); i++)
	{
		QVector4D edge = edges.at(i);
		vector<QVector2D> constraints = meshhandler->constraints(i);

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

            if((constraints.at(0).x() != 0.1f && constraints.at(0).y() != 0.1f) ||
               (constraints.at(1).x() != 0.1f && constraints.at(1).y() != 0.1f))
            {
                CanvasPointConstraint *startConstraint = new CanvasPointConstraint(startPoint, line);
                startConstraint->setPos(constraints.at(0).toPointF());

                CanvasPointConstraint *endConstraint = new CanvasPointConstraint(endPoint, line);
                endConstraint->setPos(constraints.at(1).toPointF());
            }
            else
            {
                //TODO: Fix. Occures when adding a face inside the first face. This is becuase how makeFace works
                addConstrainsForLine(startPoint, endPoint, line);
            }


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
    QString meshOld;

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
                meshOld = QString(currentMeshHandler()->saveOpenMeshAsOff().data());
                madeFace = addFaceToOpnMesh(vertsToAddFaceIdx, collidePoint);
                if(!madeFace) return;
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
            //TODO: MakeFace return faceHandle
            currentMeshHandler()->garbageCollectOpenMesh();
            addEdgesToCanvasFace(vertsToAddFaceIdx, currentMeshHandler()->numberOfFaces()-1);
            updateVertexConstraints();
            QString meshNew(currentMeshHandler()->saveOpenMeshAsOff().data());
            currentLayer()->undoStack()->push(new undoCommands::GUIChange(this, meshOld, meshNew));
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
            addConstrainsForLine(lineStartPoint_, lineEndPoint_, line);
        }
        lineStartPoint_ = lineEndPoint_;
        lineEndPoint_ = nullptr;
    }
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
    //TODO: reimplement as prepareRendering does the same, but need this function for autosaving during testing
    currentMeshHandler()->saveToTestFile();
    //vertsToAddFace_ < 1 is needed, else it will crash mesh.cpp(since the verts will have 0 valencey)
    if(renderAutoUpdate_ && vertsToAddFace_.size() < 1)
    {
        prepareRendering();
        emit GUIMeshChanged();
    }
}

void GMCanvas::resizeOpenGLWidget()
{
    //QRectF rect = currentLayer()->boundingRect();
    QGraphicsView * view = views().first();
    QRectF rect         = view->rect();
    QPointF point = view->mapToScene(rect.topLeft().toPoint());

    opengl_->widget()->setFixedSize(rect.size().toSize());
    opengl_->setPos(point);
}

void GMCanvas::setItemPointColorFromImage()
{
    QImage image = imageItem_->pixmap().toImage();
    QList<QGraphicsItem *> childlist = currentLayer()->points().childItems();
    for (int i = 0; i < childlist.size(); ++i) {
        CanvasItemPoint *point = static_cast<CanvasItemPoint *> (childlist.at(i));
        QPointF position = point->pos();
        QColor newColor(image.pixel(position.toPoint()));
        point->setColor(newColor, false);
    }
}

void GMCanvas::newDocument()
{
   clear();
   layerModel_->clear();
   for(int i = 0; i < layersAndMeshHandlers_.size(); i++){
       GMView::CanvasItemGroup * item = layersAndMeshHandlers_.at(i).first;
       layerModel_->appendRow(item);
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
            autoRenderOnMeshChanged();
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

    //TODO, if the edge is newly added
    if(edge != nullptr && currentMeshHandler()->numberOfFaces() > 0)
    {
        QString meshOld = QString(currentMeshHandler()->saveOpenMeshAsOff().data());
        int startIdx = edge->startPoint()->vertexHandleIdx();
        int endIdx = edge->endPoint()->vertexHandleIdx();
        int newIdx = currentMeshHandler()->insertVertexOnEdge(startIdx, endIdx, position, pointColor_);

        //If the insertions is apart of closing a new face
        if(vertsToAddFace_.size() != 0)
        {
            CanvasItemPoint* point = new CanvasItemPoint();
            point->setPos(event->scenePos());
            point->setVertexHandleIdx(newIdx);
            vertsToAddFace_.push_back(point);
            currentLayer()->addToGroup(point);
            vector<int> vertexIdx;
            addFaceToOpnMesh(vertexIdx, nullptr);
        }

        //TODO: Reimplement. that is: in GUI: make new edges, update face
        //For now it's easiest to construct the mesh from the meshhandler
        clearAllCurrLayer(false);
        currentMeshHandler()->garbageCollectOpenMesh();
        constructGuiFromMeshHandler();
        autoRenderOnMeshChanged();

        QString meshNew(currentMeshHandler()->saveOpenMeshAsOff().data());
        currentLayer()->undoStack()->push(new undoCommands::GUIChange(this, meshOld, meshNew));
    }
    else
    {
        //TODO: Give feedback to user that edges was not clicked(?)
    }

}

void GMCanvas::mouseMeshToolInsertion(QGraphicsSceneMouseEvent *event)
{
    if(event->button() != Qt::LeftButton) return;

	if (!currentMeshHandler()->isQuadMesh())
	{
		showMessage("Mesh must be in quads in order to use Mesh Tool.");
        return;
	}
	GUILogic::MeshHandler *meshhandler = currentMeshHandler();

    CanvasItemPoint *point = new CanvasItemPoint();
    point->setPos(event->scenePos());
    currentLayer()->addToGroup(point);

    int faceIdx = -1;

    QList<QGraphicsItem *> collidingItems = point->collidingItems();
    CanvasItemFace* face = nullptr;
    for (int i = 0; i < collidingItems.size(); ++i) {
        face = dynamic_cast<CanvasItemFace*> (collidingItems.at(i));
        if(face != nullptr)
        {
            faceIdx = face->faceIdx();
            break;
        }
    }

    //return if mouseEvent was outside any faces.
    if(faceIdx == -1)
    {
        delete point;
        return;
    }
    QString meshOld = QString(currentMeshHandler()->saveOpenMeshAsOff().data());
    int pointIdx = meshhandler->addVertex(point->pos(), pointColor_);
    point->setVertexHandleIdx(pointIdx);


    vector<CanvasItemLine*> edges = face->edgesInFace();
	//Content: 4 elements of vector<array<int,2>, one for each edge. Each array<int,2> is the Idxes of the start and end point for the next neighbour edge
    vector<vector<std::array<int, 2>>> edgesToInsertVertsOn;
	std::array<double, 4> ratios;
	int faceidx = face->faceIdx();
	
	//Content: idxes of the new verts on the edge. The "middle" vert idx is added first
    vector<int> newVerticesFirstFace;
    newVerticesFirstFace.push_back(pointIdx);

	for (int i = 0; i < edges.size(); ++i) {
		QPointF mousePos = event->scenePos();
		CanvasItemLine *edge = edges.at(i);
		int startIdx = edge->startPoint()->vertexHandleIdx();
		int endIdx = edge->endPoint()->vertexHandleIdx();
		vector<std::array<int, 2>> edgeToInsertOn;

		meshhandler->findEdgesForMeshToolInsert(startIdx, endIdx, faceidx, edgeToInsertOn);
		edgesToInsertVertsOn.push_back(edgeToInsertOn);

        qreal edgeLength = edge->line().length();
        qreal sx = point->x() - edge->startPoint()->x();
        qreal sy = point->y() - edge->startPoint()->y();

        qreal sxFromEnd = point->x() - edge->endPoint()->x();
        qreal syFromEnd = point->y() - edge->endPoint()->y();

		bool useXFromMouseClick = false;
        qreal ratio;
        if((sx*sxFromEnd) < 0) //one should be negative, the other one positive.
        {
            ratio = std::abs(sx/edgeLength);
			useXFromMouseClick = true;
        }
        else if((sy*syFromEnd) < 0)
        {
            ratio = std::abs(sy/edgeLength);
        }
        else
        {
            //In illustrator this results in two verts being added to the same edge
            showMessage("Illustrator case. Not implemented yet");
			delete point;
            return;
        }

        qDebug() << "dx:" << edgeLength << "ratio:" << ratio;

        //Find position of the vert to be added
        int pointPos = (int) edge->subdivededCurve().size()*ratio;
        qDebug() << "pos:" << pointPos << ", size:" << edge->subdivededCurve().size();
        QPointF position = edge->subdivededCurve().at(pointPos);
		useXFromMouseClick ? position.setX(mousePos.x()) : position.setY(mousePos.y());
		

        int newIdx = meshhandler->insertVertexOnEdge(startIdx,endIdx , position, pointColor_);

        newVerticesFirstFace.push_back(newIdx);
		ratios.at(i) = ratio;
    }

	//Construct faces of the first face
    meshhandler->meshToolInsertFaces(newVerticesFirstFace, true);


	for (int edge_i = 0; edge_i < edgesToInsertVertsOn.size(); edge_i++)
	{
		vector<int> newVertices;
		//Push back the vert that was inserted on the edge we now are iterating on.
		newVertices.push_back(newVerticesFirstFace.at(edge_i+1));
		for (int j = 0; j < edgesToInsertVertsOn.at(edge_i).size(); j++)
		{
			int startIdx = edgesToInsertVertsOn.at(edge_i).at(j).at(0);
			int endIdx = edgesToInsertVertsOn.at(edge_i).at(j).at(1);
			CanvasItemLine* edge = edgeBetweenPoints(startIdx, endIdx);
			//This will be wrong, since the edge can have opposite direction then the original edge. TODO:reimplement
			int pointPos = (int)edge->subdivededCurve().size()*ratios.at(edge_i);
			qDebug() << "pos:" << pointPos << ", size:" << edge->subdivededCurve().size();
			QPointF position = edge->subdivededCurve().at(pointPos);
			
			//Check if we should use the X or Y position from the mouseClick
			//TODO: implement for edge with slope. See image "BugMeshTool"
				//Will crash if the vert is added outside the face
			qreal sx = point->x() - edge->startPoint()->x();
			qreal sxFromEnd = point->x() - edge->endPoint()->x();
			(sx*sxFromEnd) < 0 ? position.setX(event->scenePos().x()) : position.setY(event->scenePos().y());

			int newIdx = meshhandler->insertVertexOnEdge(startIdx, endIdx, position, pointColor_);
			newVertices.push_back(newIdx);
		}
		try {
			meshhandler->meshToolInsertFaces(newVertices);
		}
		catch (int e)
		{
			if (e == -2) showMessage("Known bug.. Mesh tool used to add to close to edge..");
		}
	}

    meshhandler->garbageCollectOpenMesh();
    QString meshNew(currentMeshHandler()->saveOpenMeshAsOff().data());
    currentLayer()->undoStack()->push(new undoCommands::GUIChange(this, meshOld, meshNew));
    clearAllCurrLayer(false);
    constructGuiFromMeshHandler();
    autoRenderOnMeshChanged();
}

void GMCanvas::mouseRectangleTool(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(currentMeshHandler()->numberOfFaces() > 0)
        {
            showMessage("Rectangle tool can only be used when there are no other faces.");
            return;
        }
        if(lineStartPoint_ == nullptr)
        {
            CanvasItemPoint *point = new CanvasItemPoint(pointColor_);
            point->setPos(event->scenePos());
            addControlPoint(point);
            lineStartPoint_ = point;
        }
        else
        {
            QRectF rect = rectItem_->rect();
            qDebug() << rect;
            qDebug() << rect.normalized();

            std::array<CanvasItemPoint *, 4> points;
            points.at(0) = lineStartPoint_;

            vector<int> vertsIdx;
            vertsToAddFace_.push_back(lineStartPoint_);
            for(int i = 0; i < 3; ++i)
            {
                CanvasItemPoint *point = new CanvasItemPoint(pointColor_);
                if(i == 0) point->setPos(rect.topRight());
                else if(i == 1) point->setPos(rect.bottomRight());
                else if(i == 2) point->setPos(rect.bottomLeft());

                addControlPoint(point);
                points.at(i+1) = point;
                vertsToAddFace_.push_back(point);

                CanvasItemLine *line = new CanvasItemLine(points.at(i), points.at(i+1));
                currentLayer()->addToGroup(line);
                addConstrainsForLine(points.at(i), points.at(i+1), line);
            }
            CanvasItemLine *line = new CanvasItemLine(points.back(), points.front());
            currentLayer()->addToGroup(line);
            addConstrainsForLine(points.back(), points.front(), line);

            addFaceToOpnMesh(vertsIdx, points.front());
            addEdgesToCanvasFace(vertsIdx, currentMeshHandler()->numberOfFaces()-1);
            lineStartPoint_ = nullptr;
            removeItem(rectItem_);
            delete rectItem_;
            rectItem_ = nullptr;
        }
    }
}

void GMCanvas::addEdgesToCanvasFace(const vector<int> &vertsToAddFaceIdx, int faceIdx)
{
    CanvasItemFace * face = new CanvasItemFace(currentLayer(), faceIdx, currLayerIndex_);
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

void GMCanvas::resetLineToolData()
{
    vertsToAddFace_.clear();
    resetLineStartEnd();
}

bool GMCanvas::addFaceToOpnMesh(vector<int> &vertsToAddFaceIdx, CanvasItemPoint *collidePoint)
{
    bool sameStartAndEndPoint = false;
    bool faceInsideFace = false;

    if(collidePoint != vertsToAddFace_.at(0))
    {
        if(currentMeshHandler()->numberOfFaces() != 0)
        {
            if(collidePoint != nullptr) vertsToAddFace_.push_back(collidePoint);
        }
        else
        {
            showMessage("First face must be closed. Choose the first vertex to close and make face.");
            return false;
        }

        //Check if last point is connected to face, dosn't apply to first face
        if(currentMeshHandler()->vertexValence(vertsToAddFace_.back()->vertexHandleIdx()) == 0
                && currentMeshHandler()->numberOfFaces() > 0 )
        {
            //Delete the last point, that should not be a part of face.
            showMessage("Last vertex must be connected to a face", true);
            return false;
        }
    }
    else if (currentMeshHandler()->numberOfFaces() != 0)
    {
        if(vertsToAddFace_.size() == 2)
        {
            showMessage(tr("Face must consist of 3 vertices or more. Line 582"));
            return false;
        }
        sameStartAndEndPoint = true;

    }else if(vertsToAddFace_.size() == 2)
    {
        showMessage(tr("Face must consist of 3 vertices or more. Line 589"));
        return false;
    }


    //If only one point is in list, it should not be added.
    if(vertsToAddFace_.size() >= 2)
    {
        //If the face to be should be added inside an already existing face
        CanvasItemFace* collideFace = nullptr;

        vector<CanvasItemFace*> faces = currentLayer()->faces();
        for (int i = 0; i < faces.size(); i++)
        {
            if(faces.at(i)->contains(vertsToAddFace_.at(1)->pos()))
            {
                faceInsideFace = true;
                collideFace = faces.at(i);
                break;
            }
        }

        if(faceInsideFace && sameStartAndEndPoint)
        {
            showMessage(tr("Adding face inside face with same start and end is not supported yet..."), false);
            return false;
        }


        //push IDX to the vector that is passed to MeshHandler.
        for (int i = 0; i < vertsToAddFace_.size(); i++)
        {
            CanvasItemPoint *itemPoint = vertsToAddFace_.at(i);
            vertsToAddFaceIdx.push_back(itemPoint->vertexHandleIdx());

            if(i <= 1 || i == vertsToAddFace_.size() - 1 ) continue;

            //Check if verts are inside same face.
            if (collideFace != nullptr)
            {
                if(!collideFace->contains(itemPoint->pos()))
                {
                    showMessage(tr("Vertices need to be added inside same face. Line 630)"),true);
                    return false;
                }
            }
            else
            {
                QList<QGraphicsItem *> collidingItems = itemPoint->collidingItems();
                for (int j = 0; j < collidingItems.size(); ++j) {
                    CanvasItemFace* collidingFace = dynamic_cast<CanvasItemFace*>(collidingItems.at(j));
                    if(collidingFace)
                    {
                        if(collidingFace->layerId() != currLayerIndex_) continue;

                        showMessage(tr("Illegal. Some vertices are added outside, some within face. Line 640"),true);
                        return false;
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

            bool sameFace = currentMeshHandler()->isVertsOnSameFace(idxFront, idxBack);
            if(!sameFace && !boundaryVerts)
            {
                showMessage("Start and end vertex must be on same face", true);
                return false;
            }
        }

        bool sucsess;
        try{
            sucsess = currentMeshHandler()->makeFace(vertsToAddFaceIdx, faceInsideFace,sameStartAndEndPoint);
        }
        catch(int e)
        {
            if (e==-1)
            {
                showMessage("Second last point must be inside same face as first. Adjust, and select end vertex again", true);
                return false;
            }
        }

        qDebug() << "Made face?" << sucsess;

        if(!sucsess)
        {
            resetLineToolData();
            return false;
        }
    }
    else
    {
        resetLineToolData();
        return false;
    }

    vertsToAddFace_.clear();
    autoRenderOnMeshChanged();
    return true;
}

void GMCanvas::addConstrainsForLine(CanvasItemPoint *fromPoint, CanvasItemPoint *toPoint, CanvasItemLine *line)
{
    CanvasPointConstraint *startConstraint = new CanvasPointConstraint(fromPoint, line);
    startConstraint->setPos(QPointF(line->line().dx()*0.2, line->line().dy()*0.2));

    CanvasPointConstraint *endConstraint = new CanvasPointConstraint(toPoint, line);
    endConstraint->setPos(QPointF(line->line().dx()*-0.2, line->line().dy()*-0.2));
}
