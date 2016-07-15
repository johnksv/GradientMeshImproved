#include "canvasitemgroup.h"

CanvasItemGroup::CanvasItemGroup(QGraphicsItem *parent) : QGraphicsItemGroup(parent)
{

}

QString CanvasItemGroup::layerName()
{
    return layerName_;
}

void CanvasItemGroup::setLayerName(QString layerName_)
{
    this->layerName_ = layerName_;
}
