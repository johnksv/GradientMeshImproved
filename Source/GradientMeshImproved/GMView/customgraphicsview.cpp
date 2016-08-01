#include "customgraphicsview.h"
#include <QWheelEvent>
#include <QtMath>
#include <QDateTime>
#include <QtDebug>

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
    //Move scrollbars with alt and ctrl
    if(event->modifiers() != Qt::ControlModifier ){
        QGraphicsView::wheelEvent(event);
        return;
    }

    event->accept();
    if (event->delta() > 0)
    {
        if(zoomValue_ <= 300) zoomValue_ += 5;
    }
    else
    {
        if(zoomValue_  >= 5.1)zoomValue_ -= 5;
    }

    qreal scale = qExp(zoomValue_ / 150);

    QMatrix matrix;
    //Flip y-axis
    matrix.scale(scale, scale);
    setMatrix(matrix);
}
