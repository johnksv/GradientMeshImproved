#ifndef CANVASITEMGROUP_H
#define CANVASITEMGROUP_H

#include <QGraphicsItemGroup>
#include <QString>

class CanvasItemGroup : public QGraphicsItemGroup
{
public:
    CanvasItemGroup(QGraphicsItem *parent = Q_NULLPTR);
    QString layerName();
    void setLayerName(QString layerName_);


private:
    QString layerName_;
};

#endif // CANVASITEMGROUP_H
