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
    setColor();
    setAcceptHoverEvents(true);
    //setFlags(ItemIsSelectable);
}

void CanvasItemLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Nasty solution for always updating the line position accordingly to the points.
    setLine(QLineF(startPoint_->pos(), endPoint_->pos()));
    QGraphicsLineItem::paint(painter,option,widget);
}

QPainterPath CanvasItemLine::shape() const
{
    QPainterPath path;

    path.moveTo(startPoint_->pos());
    path.lineTo(endPoint_->pos());

    //Create stroke for mouseclick and tooltip detection
    QPainterPathStroker stroke;
    stroke.setWidth(10);

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

int CanvasItemLine::edgeHandleIdx()
{
    return edgeHandleIdx_;
}

void CanvasItemLine::setEdgeHandleIdx(int idx)
{
    edgeHandleIdx_ = idx;
}

QColor CanvasItemLine::color()
{
    return color_;
}

void CanvasItemLine::setColor(QColor &color)
{
    color_ = color;
    //TODO: Width from QSettings
    QPen pen(color_);
    setPen(pen);
}

CanvasItemPoint *CanvasItemLine::startPoint()
{
	return startPoint_;
}

CanvasItemPoint *CanvasItemLine::endPoint()
{
    return endPoint_;
}

void CanvasItemLine::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *setDiscontinuous = menu.addAction("Discontinuous/Hard edge");
    QAction *chooseColorAction = menu.addAction("Choose color");

    setDiscontinuous->setCheckable(true);
    setDiscontinuous->setChecked(discontinuous_);
    chooseColorAction->setEnabled(discontinuous_);

    QAction *selectedAction = menu.exec(event->screenPos());
    if(selectedAction == chooseColorAction)
    {
        QColor chosenColor = QColorDialog::getColor();
        if(chosenColor.isValid())
        {
            setColor(chosenColor);
        }
    }
    else if (selectedAction == setDiscontinuous)
    {
        discontinuous_ = setDiscontinuous->isChecked();
        if(!discontinuous_){
            //Edge no longer discontinuous, set color to black
            setColor();
        }
        else
        {
            //TODO: Should the colorpicker pop up when you choose the edge to be discontinuous?
            QColor chosenColor = QColorDialog::getColor();
            if(chosenColor.isValid())
            {
                setColor(chosenColor);
            }
        }
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
        toolTip.append("\n");
        toolTip.append("Color (RGB): ").append(QString::number(color_.red())).append(",");
        toolTip.append(QString::number(color_.green())).append(",").append(QString::number(color_.blue())).append(".");
        QToolTip::showText(event->screenPos(), toolTip);
    }
}
