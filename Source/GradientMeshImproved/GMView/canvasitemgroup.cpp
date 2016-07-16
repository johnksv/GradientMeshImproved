#include "canvasitemgroup.h"

CanvasItemGroup::CanvasItemGroup(QString layername, QGraphicsItem *parent) : QGraphicsItemGroup(parent), layerName_(layername)
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
