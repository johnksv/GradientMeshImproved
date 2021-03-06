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

    void on_actionSave_layer_triggered();

    void on_actionSave_all_layers_triggered();

    void on_actionMesh_Line_tool_triggered();

    void on_actionMesh_move_and_select_triggered();

    void on_actionImport_triggered();

    void on_actionClear_all_triggered();

    void on_layerToogleView_clicked();

    void on_layerDelete_clicked();

    void on_layerNew_clicked();

    void on_layer_listView_clicked(const QModelIndex &index);

    void on_layerMoveUp_clicked();

    void on_layerMoveDown_clicked();

    void on_layer_listView_activated(const QModelIndex &index);

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

    void on_renderSpinBoxMultiRes_valueChanged(int value);

    void on_renderRenderButton_clicked();

    void on_renderAutoRenderCheckBox_clicked(bool checked);

    void on_actionRender_GUI_faces_triggered(bool checked);

    void on_actionMesh_Mesh_Insertion_triggered();

    void on_imageColorFrom_clicked();

    void on_actionMesh_Rectangle_Tool_triggered();

    void on_glRenderButton_fullScene_clicked();

    void on_glRenderButton_viewport_clicked();

    void on_glRenderButton_boundingRect_clicked();

private:
    Ui::MainWindow *ui;
    QActionGroup *renderModeGroup_;
    QActionGroup *drawModeGroup_;
    GMView::GMCanvas *scene_;
    QAction *undoAction, *redoAction;

    void initWindowAction();
    void initSplitWidget(QWidget *parent);
    void initActionGroups();
    void initLayoutContainer();
    void initColorSelector();

    // 1 for pirmary, 2 for secondary, 3 for tertiary
    void handleColorButtonClick(int color);
    void layerMoveButtonsUpdate(const int index);
};

#endif // MAINWINDOW_H
