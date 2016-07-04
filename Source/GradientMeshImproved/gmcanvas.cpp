#include "gmcanvas.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsProxyWidget>


GMCanvas::GMCanvas(QObject * parent):
    QGraphicsScene(parent)
{
    opengl = new GMOpenGLWidget();
  //  QGraphicsProxyWidget *openGLWidget = addWidget(opengl);
//    openGLWidget->setPos(-300,-300);
//    openGLWidget->setZValue(0);
}


void GMCanvas::handleFileDialog(QString location, bool import){
    if(import){
        meshHandler.importGuiMesh(location);
    }else{
        meshHandler.saveGuiMeshOff(location);
    }
}



void GMCanvas::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(itemAt(mouseEvent->scenePos(),QTransform()) == 0 ){


        qDebug() << "Add:" << mouseEvent->scenePos().x() << ","<<mouseEvent->scenePos().y();
        GMCanvasItem *item = new GMCanvasItem(nullptr, mouseEvent->scenePos());
        prevCanvasItem = item;
        item->setZValue(10);
        addItem(item);
        update(-1000,-1000,2000,2000);
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void GMCanvas::setRenderingMode(int mode){
    renderingMode = mode;
}
void GMCanvas::setDrawingMode(drawModeCanvas drawMode){
    this->drawMode = drawMode;
}
