#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QColorDialog>
#include "GMView/canvasitemgroup.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene_ = new GMView::GMCanvas(this);
    ui->graphicsView->setScene(scene_);

    initWindowAction();
    ui->splitWidget->setVisible(false);
    ui->splitWidget->setParent(parent);

    ui->actionRender_constraints_handlers->setChecked(true);
    ui->openGLWidget->setScene(scene_);

    //TODO: implement undoStack
    undoStack = new QUndoStack(this);
    ui->undoView->setStack(undoStack);

    initActionGroups();
    initLayoutContainer();

    initColorSelector();
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initWindowAction()
{
    ui->menuWindow->addAction(ui->toolsWidget->toggleViewAction());
    ui->menuWindow->addAction(ui->colorWidget->toggleViewAction());
    ui->menuWindow->addAction(ui->undoWidget->toggleViewAction());
    ui->menuWindow->addAction(ui->splitWidget->toggleViewAction());
}

void MainWindow::initActionGroups()
{

    renderModeGroup_ = new QActionGroup(this);
    renderModeGroup_->addAction(ui->actionRender_Vertices_and_Edges);
    renderModeGroup_->addAction(ui->actionRender_multi_res_mesh);
    connect(renderModeGroup_, &QActionGroup::triggered, [=](QAction *selected) {selected->setChecked(true);});



    drawModeGroup_ = new QActionGroup(this);
    drawModeGroup_->addAction(ui->actionDraw_Line_tool);
    drawModeGroup_->addAction(ui->actionDraw_Circle_tool);
    drawModeGroup_->addAction(ui->actionDraw_move_and_select);
    connect(drawModeGroup_, &QActionGroup::triggered, [=](QAction *selected) { selected->setChecked(true);});

}

void MainWindow::initLayoutContainer()
{
    layerModel_ = new QStandardItemModel;
    ui->layer_listView->setModel(layerModel_);
    ui->layer_listView->setSelectionBehavior(QAbstractItemView::SelectRows);

    vector<GMView::CanvasItemGroup *> canvasLayers = scene_->layers();
    for(GMView::CanvasItemGroup *item : canvasLayers)
    {
        layerModel_->appendRow(item);
    }
}

void MainWindow::initColorSelector()
{
    QPalette palette = ui->colorRepresentation->palette();
    palette.setColor(QPalette::Button, Qt::black);

    ui->colorRepresentation->setFlat(true);
    ui->colorSec->setFlat(true);
    ui->colorTert->setFlat(true);

    ui->colorRepresentation->setPalette(palette);
    ui->colorSec->setPalette(palette);
    ui->colorTert->setPalette(palette);

    ui->colorRepresentation->setAutoFillBackground(true);
    ui->colorSec->setAutoFillBackground(true);
    ui->colorTert->setAutoFillBackground(true);

    ui->colorRepresentation->setShortcut(Qt::Key_1);
    ui->colorSec->setShortcut(Qt::Key_2);
    ui->colorTert->setShortcut(Qt::Key_3);
    ui->colorLockMode->setShortcut(Qt::Key_4);

}

void MainWindow::handleRenderModeGroup(QAction * action)
{
    action->setChecked(true);
}

void MainWindow::on_actionRender_constraints_handlers_triggered()
{
    scene_->setRenderConstraintHandlers(ui->actionRender_constraints_handlers->isChecked());
    scene_->update();
}

void MainWindow::on_actionRender_Vertices_and_Edges_triggered()
{
    scene_->setRenderVertsEdges(ui->actionRender_Vertices_and_Edges->isChecked());
    scene_->update();
}

void MainWindow::on_actionRender_multi_res_mesh_changed()
{
    if(ui->actionRender_multi_res_mesh->isChecked())
    {
        scene_->multiResFirstStepMesh();
        ui->actionDraw_move_and_select->setChecked(true);
        ui->actionDraw_Line_tool->setEnabled(false);
        ui->actionDraw_Circle_tool->setEnabled(false);
    }
    else
    {
        QString message("This will delete changes made on the multi res mesh. Are you sure?");
        QMessageBox::StandardButton response = QMessageBox::question(nullptr,"Multi resh mesh",message);
        if(response == QMessageBox::Yes)
        {
            scene_->resetMultiResMesh();
            ui->actionDraw_Line_tool->setEnabled(true);
            ui->actionDraw_Circle_tool->setEnabled(true);
        }
    }
    scene_->update();
}

void MainWindow::on_actionRender_Full_triggered()
{
    scene_-> setRenderingMode(GMView::renderModeCanvas::fullRender);
    scene_->update();
}

void MainWindow::on_actionDraw_Line_tool_triggered()
{
    scene_->setDrawingMode(GMView::drawModeCanvas::lineTool);
}

void MainWindow::on_actionDraw_Circle_tool_triggered()
{
    scene_->setDrawingMode(GMView::drawModeCanvas::circleTool);
}


void MainWindow::on_actionDraw_move_and_select_triggered()
{
    scene_->setDrawingMode(GMView::drawModeCanvas::move);
}

void MainWindow::on_actionExport_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,tr("Export as file"),
                                                    "", tr("OFF file (*.off)"));
    scene_->handleFileDialog(filename, false);
}

void MainWindow::on_actionImport_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Import file"),
                                                    "", tr("OFF file (*.off)"));
    scene_->handleFileDialog(filename,true);
}



void MainWindow::on_actionClear_all_triggered()
{
    scene_->clearAllCurrLayer();
}

void MainWindow::on_actionColor_Choose_triggered()
{
    handleColorButtonClick(1);
}

void MainWindow::on_actionExecuteRender_triggered()
{
    scene_->prepareRendering();
}

void MainWindow::on_layerToogleView_clicked()
{
    if(! ui->layer_listView->selectionModel()->selectedIndexes().isEmpty())
    {
        QModelIndex currentIndex  = ui->layer_listView->selectionModel()->currentIndex();
        scene_->toogleLayerVisibility(currentIndex.row());
        //TODO: Change name on layer / feedback that it is hidden/visible
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("No layers are selected. ");
        msgBox.exec();
    }
}

void MainWindow::on_layerDelete_clicked()
{
    //TODO: If only one layer
    //TODO: Update List.
    if(! ui->layer_listView->selectionModel()->selectedIndexes().isEmpty())
    {
        if(layerModel_->rowCount() == 1 )
        {
            QMessageBox msgBox;
            msgBox.setText("Can not delete base layer");
            msgBox.exec();
        }
        else
        {
            int currentIndex  = ui->layer_listView->selectionModel()->currentIndex().row();
            scene_->deleteLayer(currentIndex);
            layerModel_->removeRow(currentIndex);
        }
    }
    else
    {
        QMessageBox msgBox;
            msgBox.setText("No layers are selected. ");
            msgBox.exec();
    }
}

void MainWindow::on_layerNew_clicked()
{
    scene_->addLayer(QString("Layer " + QString::number(layerModel_->rowCount() + 1)));
    layerModel_->appendRow(scene_->layers().back());
}

void MainWindow::on_layer_listView_clicked(const QModelIndex &index)
{
    scene_->setActiveLayer(index.row());
}


void MainWindow::on_actionRender_in_split_window_changed()
{
    bool checked = ui->actionRender_in_split_window->isChecked();
    scene_->drawOpenGlOnCanvas(!checked);
}

void MainWindow::on_colorRepresentation_clicked()
{
    handleColorButtonClick(1);
}

void MainWindow::on_colorSec_clicked()
{
    handleColorButtonClick(2);
}

void MainWindow::on_colorTert_clicked()
{
    handleColorButtonClick(3);
}
void MainWindow::handleColorButtonClick(int color)

{
    // 1 for pirmary, 2 for secondary, 3 for tertiary
    QPushButton *button;
    if(color == 1){
        button = ui->colorRepresentation;
    }else if(color == 2){
        button = ui->colorSec;
    }else {
        button = ui->colorTert;
    }

    QPalette palette = button->palette();

    if( !ui->colorLockMode->isChecked()){
        QColor chosenColor = QColorDialog::getColor();
        if(chosenColor.isValid())
        {
            palette.setColor(QPalette::Button, chosenColor);
        }
    }

    scene_->setDrawColorVertex(palette.color(QPalette::Button));

    button->setAutoFillBackground(true);
    button->setPalette(palette);
    button->update();
}

void MainWindow::on_actionClearSelection_triggered()
{
    scene_->resetLineStartEnd();
    scene_->clearSelection();
}

void MainWindow::on_actionHelpAbout_Qt_triggered()
{
     QApplication::aboutQt();
}
