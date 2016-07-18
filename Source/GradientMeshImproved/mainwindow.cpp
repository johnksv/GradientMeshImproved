#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QColorDialog>
#include "GMView/canvasitemgroup.h"
#include <QStandardItemModel>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene_ = new GMCanvas(this);
    ui->graphicsView->setScene(scene_);
    initActionGroups();

    initLayoutContainer();

    ui->colorRepresentation->setFlat(true);
    ui->colorRepresentation->setStyleSheet(QString("QPushButton {"
    "background-color: #000000}"));
    ui->colorRepresentation->setAutoFillBackground(true);
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initActionGroups()
{
    renderModeGroup_ = new QActionGroup(this);
    renderModeGroup_->addAction(ui->actionRender_Vertices_only);
    renderModeGroup_->addAction(ui->actionRender_Vertices_and_Edges);
    renderModeGroup_->addAction(ui->actionRender_Partial);
    renderModeGroup_->addAction(ui->actionRender_Full);
    connect(renderModeGroup_, SIGNAL(triggered(QAction*)), this, SLOT(handleRenderModeGroup(QAction*)));

    drawModeGroup_ = new QActionGroup(this);

}

void MainWindow::initLayoutContainer()
{
    layerModel_ = new QStandardItemModel;
    ui->layer_listView->setModel(layerModel_);
    ui->layer_listView->setSelectionBehavior(QAbstractItemView::SelectRows);

    vector<CanvasItemGroup *> canvasLayers = scene_->layers();
    for(CanvasItemGroup *item : canvasLayers)
    {
        layerModel_->appendRow(item);
    }
}

void MainWindow::handleRenderModeGroup(QAction * action)
{
    action->setChecked(true);
}

void MainWindow::on_actionRender_Vertices_only_triggered()
{
//    ui->openGLWidget-> setRenderingMode(0);
//    ui->openGLWidget-> paintGL();
}

void MainWindow::on_actionRender_Vertices_and_Edges_triggered()
{
//    ui->openGLWidget-> setRenderingMode(1);
//    ui->openGLWidget-> paintGL();
}

void MainWindow::on_actionRender_Partial_triggered()
{
//    ui->openGLWidget-> setRenderingMode(2);
//    ui->openGLWidget-> paintGL();
}

void MainWindow::on_actionRender_Full_triggered()
{
//    ui->openGLWidget-> setRenderingMode(3);
//    ui->openGLWidget-> paintGL();
}

void MainWindow::on_actionDraw_Line_tool_triggered()
{
    scene_->setDrawingMode(drawModeCanvas::vertAndEdge);
}

void MainWindow::on_actionMove_triggered()
{
    scene_->setDrawingMode(drawModeCanvas::move);
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
    scene_->clearAll();
}

void MainWindow::on_actionColor_Choose_triggered()
{
    QColor chosenColor = QColorDialog::getColor();
    if(chosenColor.isValid())
    {
        scene_->setDrawColorVertex(chosenColor);

        QString css = QString("QPushButton {background-color: %1}").arg(chosenColor.name());
        ui->colorRepresentation->setStyleSheet(css);
        ui->colorRepresentation->setAutoFillBackground(true);
    }
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

void MainWindow::on_colorRepresentation_clicked()
{
    on_actionColor_Choose_triggered();
}

void MainWindow::on_toolsWidget_visibilityChanged(bool visible)
{
        ui->actionToolsWidget->setChecked(visible);
}

void MainWindow::on_actionToolsWidget_changed()
{
    if( ui->actionToolsWidget->isChecked())
    {
         ui->toolsWidget->setVisible(true);
    }
    else
    {
        ui->toolsWidget->setVisible(false);
    }
}
