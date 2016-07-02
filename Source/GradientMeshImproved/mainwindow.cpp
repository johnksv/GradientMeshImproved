#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initActionGroups();
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
    connect(renderModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(handleRenderModeGroup));

    drawModeGroup = new QActionGroup(this);

}

void MainWindow::handleRenderModeGroup(QAction * action){
    action->setChecked(true);
}

void MainWindow::on_actionRender_Vertices_only_triggered()
{
    ui->openGLWidget-> setRenderingMode(0);
    ui->openGLWidget-> paintGL();
}

void MainWindow::on_actionRender_Vertices_and_Edges_triggered()
{
    ui->openGLWidget-> setRenderingMode(1);
    ui->openGLWidget-> paintGL();
}

void MainWindow::on_actionRender_Partial_triggered()
{
    ui->openGLWidget-> setRenderingMode(2);
    ui->openGLWidget-> paintGL();
}

void MainWindow::on_actionRender_Full_triggered()
{
    ui->openGLWidget-> setRenderingMode(3);
    ui->openGLWidget-> paintGL();
}

void MainWindow::on_actionExport_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,tr("Export as file"),
                                                    "", tr("OFF file (*.off)"));
    ui->openGLWidget->handleFileDialog(filename, false);
}

void MainWindow::on_actionDraw_Line_tool_triggered()
{
    ui->openGLWidget->setDrawingMode(drawModeCanvas::vertAndEdge);
}

void MainWindow::on_actionImport_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Import file"),
                                                    "", tr("OFF file (*.off)"));
    ui->openGLWidget->handleFileDialog(filename,true);
}
