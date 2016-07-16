#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include "GMView/canvas.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionRender_Vertices_only_triggered();

    void on_actionRender_Full_triggered();

    void on_actionRender_Vertices_and_Edges_triggered();

    void on_actionRender_Partial_triggered();

    void on_actionExport_triggered();

    void on_actionDraw_Line_tool_triggered();

    void on_actionMove_triggered();

    void on_actionImport_triggered();

    void handleRenderModeGroup(QAction * action);

    void on_actionClear_all_triggered();

    void on_actionColor_Choose_triggered();

    void on_actionExecuteRender_triggered();

    void on_layerToogleView_clicked();

    void on_layerDelete_clicked();

    void on_layerNew_clicked();

    void on_layer_listView_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    void initActionGroups();
    QActionGroup *renderModeGroup_;
    QActionGroup *drawModeGroup_;
    GMCanvas *scene_;
    QStandardItemModel *layerModel_;
};

#endif // MAINWINDOW_H
