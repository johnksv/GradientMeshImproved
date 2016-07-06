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

    void on_actionVertices_triggered();

    void on_actionImport_triggered();

    void handleRenderModeGroup(QAction * action);

private:
    Ui::MainWindow *ui;
    void initActionGroups();
    QActionGroup *renderModeGroup;
    QActionGroup *drawModeGroup;
    GMCanvas *scene;
};

#endif // MAINWINDOW_H
