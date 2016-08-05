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

using namespace GMView;

CanvasItemLine::CanvasItemLine(CanvasItemPoint *startPoint, CanvasItemPoint *endPoint, QGraphicsItem *parent) : QGraphicsLineItem(parent), startPoint_(startPoint), endPoint_(endPoint)
{
    setLine(QLineF(startPoint->pos(), endPoint->pos()));
    setZValue(1);
    setAcceptHoverEvents(true);
    //setFlags(ItemIsSelectable);
}

void CanvasItemLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Nasty solution for always updating the line position accordingly to the points.
    setLine(QLineF(startPoint_->pos(), endPoint_->pos()));

    //For drawing the subdivided line
    for (int i = 1; i < subdividedCurve_.size(); i++){
        painter->drawLine(subdividedCurve_.at(i-1),subdividedCurve_.at(i));
    }

    //For drawing orginal line
    QGraphicsLineItem::paint(painter,option,widget);
}

QPainterPath CanvasItemLine::shape() const
{
    QPainterPath path;

    path.moveTo(startPoint_->pos());

    path.lineTo(endPoint_->pos());
    for (int i = 1; i < subdividedCurve_.size(); i++){
    //   path.lineTo(subdividedCurve_.at(i));
    }

    //Create stroke for mouseclick and tooltip detection
    QPainterPathStroker stroke;
    stroke.setWidth(5);

    return stroke.createStroke(path);
}


bool CanvasItemLine::operator ==(const CanvasItemLine &lineA)
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

CanvasItemPoint *CanvasItemLine::startPoint()
{
	return startPoint_;
}

CanvasItemPoint *CanvasItemLine::endPoint()
{
    return endPoint_;
}

void CanvasItemLine::updateSubdivisonCurve()
{
    QGraphicsItem *startConstraint = startPoint_->controlPoint(this);
    QGraphicsItem *endConstraint = endPoint_->controlPoint(this);
    vector<QPointF> inputPoints;
    inputPoints.push_back(startPoint_->pos());
    //TODO: Remove. startConstraint and  endConstraint should never be nullptrs.
    if(startConstraint != nullptr && endConstraint != nullptr)
    {
        inputPoints.push_back(startConstraint->mapToParent(startConstraint->pos()));
        inputPoints.push_back(endConstraint->mapToParent(endConstraint->pos()));
    }
    inputPoints.push_back(endPoint_->pos());

    subdividedCurve_ = subdCurve(inputPoints,4);
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
            startPoint_->setDiscontinuous(value);
            endPoint_->setDiscontinuous(value);
        }
}

void CanvasItemLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    GMCanvas* parent = static_cast <GMCanvas*> (scene());
    if(parent->drawingMode() == drawModeCanvas::move)
    {
        QString toolTip("Line:");
        toolTip.append("\n");
        toolTip.append("Discontinuous: ");
        discontinuous_ ? toolTip.append("yes.") : toolTip.append("no.");

        QToolTip::showText(event->screenPos(), toolTip);
    }
}

//Check that actions on points perform as excpected. Edit: they are not..
//TODO: Fix subdCurve
vector<QPointF> CanvasItemLine::subdCurve(vector<QPointF> spline, int steps, bool closed)
{
    // recursive halting condition
    if(steps==0)
        return spline;

    // allocate new polygon on stack
    vector<QPointF> newVec;

    // SPECIAL CASES: open curve: start at end point; start at end point if closed and
    // number of vertices is below 4
    if(!closed||spline.size()<4) // size or size method
        newVec.push_back(spline.front());

    // SPECIAL CASE: deal with splines with less than 4 control points
    if(spline.size()==2) { // create a line (between the two points)
        newVec.push_back(0.667*spline.front()+0.333*spline.back());
        newVec.push_back(0.333*spline.front()+0.667*spline.back());
        newVec.push_back(spline.back());
        return subdCurve(newVec,steps-1, closed);
    }

    // three points
    if(spline.size()==3) {
        newVec.push_back(0.5*spline.front()+0.5*spline.at(1));
        newVec.push_back(0.125*spline.front()+0.75*spline.at(1) + 0.125*spline.back());
        newVec.push_back(0.5*spline.back()+0.5*spline.at(1));

        newVec.push_back(spline.back());
//        if (spline.at(1).isSharp)
//        {
//            newVec[newVec.size()/2] = spline.at(1);
//        }
        return subdCurve(newVec,steps-1, closed);
    }

    // Perform subdivision for the standard case.
    for(int i = 1;i<spline.size()-1;i++)
    {
        QPointF new1 = 0.5*spline.at(i-1)+0.5*spline.at(i);
        QPointF new2 = 0.125*spline.at(i-1)+0.75*spline.at(i)+0.125*spline.at(i+1);
//        if (spline.at(i).isSharp) // if control point is set to "sharp" (ignore if feature is not supported)
//        {
//            new2 = spline.at(i);
//        }
        newVec.push_back(new1);
        newVec.push_back(new2);
    }

    // add second last (open) or last (closed) control point
    newVec.push_back(0.5*spline.at(spline.size()-2)+0.5*spline.back());

    // if open, add last point
    if(!closed)
        newVec.push_back(spline.back());

    // recursively perform the next subdivision step
    return subdCurve(newVec,steps-1, closed);
}
