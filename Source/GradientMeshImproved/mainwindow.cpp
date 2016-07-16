#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QColorDialog>
#include "GMView/canvasitemgroup.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene = new GMCanvas(this);
    ui->graphicsView->setScene(scene);
    initActionGroups();


    vector<CanvasItemGroup *> canvasLayers = scene->layers();
    for(auto item : canvasLayers)
    {
        ui->layerListWidget->addItem(item->layerName());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initActionGroups(){
    renderModeGroup = new QActionGroup(this);
    renderModeGroup->addAction(ui->actionRender_Vertices_only);
    renderModeGroup->addAction(ui->actionRender_Vertices_and_Edges);
    renderModeGroup->addAction(ui->actionRender_Partial);
    renderModeGroup->addAction(ui->actionRender_Full);
    connect(renderModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(handleRenderModeGroup(QAction*)));

    drawModeGroup = new QActionGroup(this);

}

void MainWindow::handleRenderModeGroup(QAction * action){
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
    scene->setDrawingMode(drawModeCanvas::vertAndEdge);
}

void MainWindow::on_actionMove_triggered()
{
    scene->setDrawingMode(drawModeCanvas::move);
}

void MainWindow::on_actionExport_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,tr("Export as file"),
                                                    "", tr("OFF file (*.off)"));
    scene->handleFileDialog(filename, false);
}

void MainWindow::on_actionImport_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Import file"),
                                                    "", tr("OFF file (*.off)"));
    scene->handleFileDialog(filename,true);
}



void MainWindow::on_actionClear_all_triggered()
{
    scene->clearAll();
}

void MainWindow::on_actionColor_Choose_triggered()
{
    QColorDialog *colordialog = new QColorDialog();
    colordialog->open();

    QObject::connect(colordialog, SIGNAL(colorSelected(QColor)), scene, SLOT(setDrawColorVertex(QColor)));
}

void MainWindow::on_actionExecuteRender_triggered()
{
    scene->prepareRendering();
}

void MainWindow::on_layerToogleView_clicked()
{

}

void MainWindow::on_layerDelete_clicked()
{

}
