#ifndef CANVASITEMGROUP_H
#define CANVASITEMGROUP_H

#include <QGraphicsItemGroup>
#include <QStandardItem>
#include <QString>
#include "canvasitemline.h"
#include "canvasitemface.h"


class CanvasItemGroup : public QGraphicsItemGroup, public QStandardItem
{
public:
    CanvasItemGroup(QString layername, QGraphicsItem *parent = Q_NULLPTR);
    friend class GMCanvas;

private:
    vector<CanvasItemPoint*> points;
    vector<CanvasItemPoint*> points_selected;
    vector<CanvasItemLine*> lines;
    vector<CanvasItemFace*> faces;

};

#endif // CANVASITEMGROUP_H
