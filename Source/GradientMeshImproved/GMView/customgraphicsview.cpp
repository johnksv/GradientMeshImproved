#include "customgraphicsview.h"
#include <QWheelEvent>
#include <QtMath>
#include <QDateTime>
#include <QtDebug>
#include "GMView/canvas.h"

using namespace GMView;

CustomGraphicsView::CustomGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    setMouseTracking(true);
    setRenderHints(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    //Zoom in on mouse
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

void CustomGraphicsView::wheelEvent(QWheelEvent *event)
{
    static_cast<GMCanvas *> (scene())->resizeOpenGLWidget(viewport()->size());
    //Move scrollbars with alt and ctrl
    if(event->modifiers() != Qt::ControlModifier ){
        QGraphicsView::wheelEvent(event);
        return;
    }
    event->accept();

    if (event->angleDelta().y() > 0)
    {
        if(zoomValue_ <= 500) zoomValue_ += 5;
    }
    else
    {
        if(zoomValue_  >= 100)zoomValue_ -= 5;
    }


    qreal scale = qPow(2, (zoomValue_ - 250) / qreal(50));
    if( width()/scale > sceneRect().width())
    {
        fitInView(sceneRect(), Qt::KeepAspectRatio);
    }else
    {
        QMatrix matrix;
        matrix.scale(scale, scale);
        setMatrix(matrix);
    }
}

void CustomGraphicsView::resizeEvent(QResizeEvent *event)
{
    static_cast<GMCanvas *> (scene())->resizeOpenGLWidget(event->size());
}
