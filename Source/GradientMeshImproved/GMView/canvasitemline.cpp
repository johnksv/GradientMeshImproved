#include "canvasitemline.h"
#include <QDebug>
#include <QGraphicsSceneContextMenuEvent>
#include <QAction>
#include <QMenu>
#include <QColorDialog>
#include <QPainter>
#include <QPen>
#include <QPainterPath>
#include <QToolTip>
#include "canvas.h"
#include "GMView/utils.h"
#include <QStyleOptionGraphicsItem>
#include <QMessageBox>

using namespace GMView;

CanvasItemLine::CanvasItemLine(CanvasItemPoint *startPoint, CanvasItemPoint *endPoint, QGraphicsItem *parent) : QGraphicsLineItem(parent), startPoint_(startPoint), endPoint_(endPoint)
{
    setLine(QLineF(startPoint->pos(), endPoint->pos()));

    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

void CanvasItemLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const qreal detailLevel = option->levelOfDetailFromTransform(painter->worldTransform());
    double width = 1 / detailLevel;
    painter->setPen(QPen(Qt::black,width));

    //For drawing the subdivided line
    for (int i = 1; i < subdividedCurve_.size(); i++){
        painter->drawLine(subdividedCurve_.at(i-1),subdividedCurve_.at(i));
    }

    //For drawing orginal line
    //QGraphicsLineItem::paint(painter,option,widget);
}

QPainterPath CanvasItemLine::shape() const
{
    QPainterPath path;

    path.moveTo(startPoint_->pos());
    for (int i = 1; i < subdividedCurve_.size(); i++){
       path.lineTo(subdividedCurve_.at(i));
    }

    //Create stroke for mouseclick and tooltip detection
    QPainterPathStroker stroke;
    stroke.setWidth(5);

    return stroke.createStroke(path);
}


bool CanvasItemLine::operator ==(const CanvasItemLine &lineA) const
{
    if((*this).startPoint_ == lineA.startPoint_)
    {
        if((*this).endPoint_ == lineA.endPoint_)
        {
            return true;
        }
    }
    return false;
}

CanvasItemPoint *CanvasItemLine::startPoint() const
{
    return startPoint_;
}

CanvasItemPoint *CanvasItemLine::endPoint() const
{
    return endPoint_;
}

void CanvasItemLine::updateSubdivisonCurve()
{
    setLine(QLineF(startPoint_->pos(), endPoint_->pos()));

    QGraphicsItem *startConstraint = startPoint_->constraintPoint(this);
    QGraphicsItem *endConstraint = endPoint_->constraintPoint(this);
    vector<QPointF> inputPoints;
    inputPoints.push_back(startPoint_->pos());

    if(startConstraint != nullptr && endConstraint != nullptr)
    {
        inputPoints.push_back(startConstraint->mapToScene(startConstraint->pos())-startConstraint->pos());
        inputPoints.push_back(endConstraint->mapToScene(endConstraint->pos())-endConstraint->pos());
    }
    inputPoints.push_back(endPoint_->pos());


    subdividedCurve_ = GMView::subdCurve(inputPoints,3);
}

const vector<QPointF> &CanvasItemLine::subdivededCurve() const
{
    return subdividedCurve_;
}

void CanvasItemLine::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    GMCanvas *canvas = static_cast<GMCanvas*> (scene());
    int startIdx = startPoint()->vertexHandleIdx();
    int endIdx = endPoint()->vertexHandleIdx();

    QMenu menu;
    QAction *setDiscontinuous = menu.addAction("Discontinuous/Hard edge");

    setDiscontinuous->setCheckable(true);
    setDiscontinuous->setChecked(discontinuous_);
    if(canvas->currentMeshHandler()->isBoundaryEdge(startIdx, endIdx))
    {
        setDiscontinuous->setEnabled(false);
    }

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == setDiscontinuous)
    {
        bool value = setDiscontinuous->isChecked();
        discontinuous_ = value;

        if(discontinuous_)
        {
        }
        else
        {
        }
    }
}

void CanvasItemLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
}
