#include "mainwindow.h"
#include "ui_mainwindow.h"

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

void MainWindow::on_actionFull_render_Change_name_triggered()
{
    ui->openGLWidget-> setRenderingMode(1);
    ui->openGLWidget-> paintGL();
}
