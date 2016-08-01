#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QGraphicsView>

namespace GMView{

class CustomGraphicsView : public QGraphicsView
{
public:
    CustomGraphicsView(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    qreal zoomValue_ = 0;
};

} // end of namespace GUIView

#endif // CUSTOMGRAPHICSVIEW_H
