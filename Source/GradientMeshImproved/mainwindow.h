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
    void on_actionRender_constraints_handlers_triggered();

    void on_actionRender_multi_res_mesh_changed();

    void on_actionRender_auto_toggled(bool checked);

    void on_actionExport_triggered();

    void on_actionMesh_Line_tool_triggered();

    void on_actionMesh_move_and_select_triggered();

    void on_actionImport_triggered();

    void handleRenderModeGroup(QAction * action);

    void on_actionClear_all_triggered();

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

    void on_actionMesh_Insert_on_Edge_triggered();

    void on_actionMesh_Collapse_Edge_triggered();

    void on_imageImport_clicked();

    void on_imageRemove_clicked();

    void on_imageOpacity_valueChanged(int value);

    void on_action_New_triggered();

    void on_renderSpinBox_valueChanged(int value);

    void on_renderRenderButton_clicked();

    void on_renderAutoRenderCheckBox_clicked(bool checked);

    void on_actionRender_GUI_faces_triggered(bool checked);

    void on_actionMesh_Knot_Insertion_triggered();

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
