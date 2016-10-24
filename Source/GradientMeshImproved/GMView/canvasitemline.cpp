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

    setZValue(1);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    //setFlags(ItemIsSelectable);
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
    return;
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
            //Discontinued points are created
            startPoint_->setDiscontinuous(value, this);
            endPoint_->setDiscontinuous(value, this);

            vector<int> disconVertIdx;

            int disPointIdx1 = static_cast<CanvasPointDiscontinued*> (startPoint_->discontinuedChildren().at(1))->vertexHandleIdx();
            int disPointIdx2 = static_cast<CanvasPointDiscontinued*> (endPoint_->discontinuedChildren().at(1))->vertexHandleIdx();


            disconVertIdx.push_back(startPoint_->vertexHandleIdx());
            disconVertIdx.push_back(disPointIdx1);
            disconVertIdx.push_back(disPointIdx2);
            disconVertIdx.push_back(endPoint_->vertexHandleIdx());

            try{
                qDebug() << canvas->currentMeshHandler()->makeFace(disconVertIdx, true);
            }
            catch(int e)
            {
                QMessageBox msgBox;
                msgBox.setText("Bug. Sorry. In backlog to be fixed");
                msgBox.exec();
            }
        }
        else
        {
            vector<int> disconVertIdx;

            int disPointIdx1 = static_cast<CanvasPointDiscontinued*> (startPoint_->discontinuedChildren().at(1))->vertexHandleIdx();
            int disPointIdx2 = static_cast<CanvasPointDiscontinued*> (endPoint_->discontinuedChildren().at(1))->vertexHandleIdx();


            disconVertIdx.push_back(startPoint_->vertexHandleIdx());
            disconVertIdx.push_back(disPointIdx1);
            disconVertIdx.push_back(disPointIdx2);
            disconVertIdx.push_back(endPoint_->vertexHandleIdx());

            canvas->currentMeshHandler()->deleteDiscontinuedFace(disconVertIdx);

            //Discontinued are deleted in canvasPoint
            startPoint_->setDiscontinuous(value, this);
            endPoint_->setDiscontinuous(value, this);

        }
    }
}

void CanvasItemLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
}
