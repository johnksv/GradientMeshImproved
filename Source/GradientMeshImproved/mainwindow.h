#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include "GMView/canvas.h"
#include <QUndoStack>

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
    void on_actionRender_no_constraints_handlers_triggered();

    void on_actionRender_Full_triggered();

    void on_actionRender_Vertices_and_Edges_triggered();

    void on_actionRender_Partial_triggered();

    void on_actionExport_triggered();

    void on_actionDraw_Line_tool_triggered();

    void on_actionDraw_move_and_select_triggered();

    void on_actionImport_triggered();

    void handleRenderModeGroup(QAction * action);

    void on_actionClear_all_triggered();

    void on_actionColor_Choose_triggered();

    void on_actionExecuteRender_triggered();

    void on_layerToogleView_clicked();

    void on_layerDelete_clicked();

    void on_layerNew_clicked();

    void on_layer_listView_clicked(const QModelIndex &index);

    void on_colorRepresentation_clicked();

    void on_actionRender_in_split_window_changed();


    void on_colorSec_clicked();

    void on_colorTert_clicked();

    void on_actionClearSelection_triggered();

    void on_actionHelpAbout_Qt_triggered();

private:
    Ui::MainWindow *ui;
    void initActionGroups();
    QActionGroup *renderModeGroup_;
    QActionGroup *drawModeGroup_;
    GMView::GMCanvas *scene_;
    QStandardItemModel *layerModel_;
    QUndoStack *undoStack;
    void initLayoutContainer();
    void initColorSelector();
    // 1 for pirmary, 2 for secondary, 3 for tertiary
    void handleColorButtonClick(int color);
    void initWindowAction();
};

#endif // MAINWINDOW_H
