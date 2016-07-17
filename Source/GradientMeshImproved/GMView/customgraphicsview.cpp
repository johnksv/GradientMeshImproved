#include "customgraphicsview.h"
#include <QWheelEvent>
#include <QtDebug>
CustomGraphicsView::CustomGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    setMouseTracking(true);
    setRenderHints(QPainter::Antialiasing);

}

void CustomGraphicsView::wheelEvent(QWheelEvent *event)
{
    if(event->modifiers() == Qt::AltModifier ||
           event->modifiers() == Qt::ControlModifier ){
        QGraphicsView::wheelEvent(event);
        return;
    }
    //Zoom in on mouse
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    if(event->delta()>0){
        scale(scaleFactor, scaleFactor);
    }else{
        //Zoom out
        scale(1/scaleFactor, 1/scaleFactor);
    }
}
