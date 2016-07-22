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
#include <QInputDialog>
#include "canvas.h"


CanvasItemPoint::CanvasItemPoint(QColor color, QGraphicsItem *parent):
    QGraphicsItem(parent), color_(color)
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);

}

QRectF CanvasItemPoint::boundingRect() const
{
    return QRectF(-radius_, -radius_,radius_*2,radius_*2);
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
    painter->drawEllipse(boundingRect());
}

QPainterPath CanvasItemPoint::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}


QColor CanvasItemPoint::color()
{
    return color_;
}

double CanvasItemPoint::weight()
{
    return weight_;
}

void CanvasItemPoint::setRadius(int _radius)
{
    if(_radius<=0){
        radius_ = 1;
    }else{
        radius_ = _radius;
    }
}

GUILogic::OpnMeshVertHandle CanvasItemPoint::vertexHandle()
{
    return vertexHandle_;
}

void CanvasItemPoint::setVertexHandle(GUILogic::OpnMeshVertHandle vertexHandle)
{
    vertexHandle_ = vertexHandle;
}

QVariant CanvasItemPoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemScenePositionHasChanged)
    {
        GMCanvas* parent = static_cast <GMCanvas*> (scene());
        parent->updateVertexFromPoint(*this, 0);
    }
    return QGraphicsItem::itemChange(change, value);
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
    menu.addSeparator();
    QAction *deletePointAction = menu.addAction("Delete");
    QAction *selectedAction = menu.exec(event->screenPos());
    if(selectedAction == chooseColorAction)
    {
        QColorDialog *colordialog = new QColorDialog(color_);
        colordialog->open();

        QObject::connect(colordialog, SIGNAL(colorSelected(QColor)), this, SLOT(setColor(QColor)));
    }
    else if(selectedAction == setWeightAction)
    {
        QInputDialog *weightDialog = new QInputDialog();
        weightDialog->setInputMode(QInputDialog::DoubleInput);
        weightDialog->setDoubleMaximum(1);
        weightDialog->setLabelText("Enter a value (0.00 - 1.00):");
        weightDialog->open();

        QObject::connect(weightDialog, SIGNAL(doubleValueSelected(double)), this, SLOT(setWeight(double)));
    }
    else if(selectedAction == deletePointAction)
    {
        //TODO Implement delete point
    }
}

void CanvasItemPoint::setColor(QColor color)
{
    this->color_ = color;
    GMCanvas* parent = static_cast <GMCanvas*> (scene());
    parent->updateVertexFromPoint(*this, 1);
    update(boundingRect());
}

void CanvasItemPoint::setWeight(double weight)
{
    this->weight_ = weight;
    GMCanvas* parent = static_cast <GMCanvas*> (scene());
    parent->updateVertexFromPoint(*this, 2);
}
