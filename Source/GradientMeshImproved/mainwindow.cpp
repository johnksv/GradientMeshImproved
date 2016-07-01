#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionVertices_only_triggered()
{
    ui->openGLWidget-> setRenderingMode(0);
    ui->openGLWidget-> paintGL();
}

void MainWindow::on_actionVertices_and_Edges_triggered()
{
    ui->openGLWidget-> setRenderingMode(1);
    ui->openGLWidget-> paintGL();
}

void MainWindow::on_actionPartial_render_triggered()
{
    ui->openGLWidget-> setRenderingMode(2);
    ui->openGLWidget-> paintGL();
}

void MainWindow::on_actionFull_render_triggered()
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
