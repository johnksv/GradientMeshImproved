#include "canvaspoint.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QAction>
#include <QMenu>
#include <QVariant>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QColorDialog>
#include "canvas.h"


CanvasItemPoint::CanvasItemPoint(QPointF pos, QGraphicsItem *parent):
    QGraphicsItem(parent), position_(pos)
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsMovable |ItemIsSelectable |ItemSendsScenePositionChanges );
}

QRectF CanvasItemPoint::boundingRect() const
{
    return QRectF(position_.x()-radius_, position_.y()-radius_,radius_*2,radius_*2);
}

void CanvasItemPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if(isSelected()){
        //TODO: Fix selected color
        QColor temp = QColor(color_);
        temp.setAlpha(150);
        painter->setBrush(temp);
    }else{
        painter->setBrush(color_);
    }
    if(hovered_){
        painter->setOpacity(0.5);
    }else{
        painter->setOpacity(1);
    }
    painter->drawEllipse(position_,radius_,radius_);
}

QPainterPath CanvasItemPoint::shape() const
{
    QPainterPath path;
    path.addEllipse(position_,radius_,radius_);
    return path;
}

QVariant CanvasItemPoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemScenePositionHasChanged){
        qDebug() << "CanvasPoint 48:  Item moved";
    }
    return QGraphicsItem::itemChange(change, value);
}

QPointF CanvasItemPoint::position()
{
    return position_;
}

QColor CanvasItemPoint::color()
{
    return color_;
}

void CanvasItemPoint::setRadius(int _radius)
{
    if(_radius<=0){
        radius_ = 1;
    }else{
        radius_ = _radius;
    }
}

void CanvasItemPoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

}

void CanvasItemPoint::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
    hovered_ = true;
    update();
}

void CanvasItemPoint::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
    hovered_ = false;
    update();
}

void CanvasItemPoint::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *chooseColorAction = menu.addAction("Choose color");
    QAction *setWeightAction = menu.addAction("Set weight");
    QAction *selectedAction = menu.exec(event->screenPos());
    if(selectedAction == chooseColorAction)
    {
        QColorDialog *colordialog = new QColorDialog(color_);
        colordialog->open();

        QObject::connect(colordialog, SIGNAL(colorSelected(QColor)), this, SLOT(setColor(QColor)));
    }
}

void CanvasItemPoint::setColor(QColor color)
{
    this->color_ = color;
    GMCanvas* parent = static_cast <GMCanvas*> (scene());
    parent->updateColorVertex(this);
    update(boundingRect());
}
