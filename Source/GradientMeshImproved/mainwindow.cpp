#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QColorDialog>
#include "GMView/canvasitemgroup.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QApplication>
#include <QImageReader>
#include <QByteArrayList>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene_ = new GMView::GMCanvas(this);
    ui->graphicsView->setScene(scene_);

    initWindowAction();
    initSplitWidget(parent);

    ui->actionRender_constraints_handlers->setChecked(true);
    ui->openGLWidget->setScene(scene_);
    connect(scene_, &GMView::GMCanvas::GUIMeshChanged, [=] ()
    { if(ui->openGLWidget->isVisible())
        {
            ui->openGLWidget->update();
        }
    });

    //TODO: implement undoStack
    undoAction = scene_->currentLayer()->undoStack()->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcuts(QKeySequence::Undo);
    ui->menuEdit->addAction(undoAction);
    redoAction = scene_->currentLayer()->undoStack()->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcuts(QKeySequence::Redo);
    ui->menuEdit->addAction(redoAction);

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
    ui->menuWindow->addAction(ui->renderWidget->toggleViewAction());
    ui->menuWindow->addAction(ui->toolsWidget->toggleViewAction());
    ui->menuWindow->addAction(ui->colorEasyWidget->toggleViewAction());
    ui->menuWindow->addAction(ui->colorWidget->toggleViewAction());
    ui->menuWindow->addAction(ui->imageWidget->toggleViewAction());
    ui->menuWindow->addAction(ui->splitWidget->toggleViewAction());

}

void MainWindow::initSplitWidget(QWidget *parent)
{
    ui->splitWidget->setVisible(false);
    ui->splitWidget->setParent(parent);
    ui->splitWidget->setMinimumSize(400,400);
    ui->splitWidget->resize(600,600);
    ui->splitWidget->setWindowFlags(Qt::WindowStaysOnTopHint);
}

void MainWindow::initActionGroups()
{
    drawModeGroup_ = new QActionGroup(this);
    drawModeGroup_->addAction(ui->actionMesh_Line_tool);
    drawModeGroup_->addAction(ui->actionMesh_move_and_select);
    drawModeGroup_->addAction(ui->actionMesh_Collapse_Edge);
    drawModeGroup_->addAction(ui->actionMesh_Insert_on_Edge);
    drawModeGroup_->addAction(ui->actionMesh_Mesh_Insertion);
    drawModeGroup_->addAction(ui->actionMesh_Rectangle_Tool);
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

void MainWindow::on_actionRender_constraints_handlers_triggered()
{
    scene_->setRenderConstraintHandlers(ui->actionRender_constraints_handlers->isChecked());
    scene_->update();
}

void MainWindow::on_actionRender_auto_toggled(bool checked)
{
    scene_->setRenderAuto(checked);
    ui->renderAutoRenderCheckBox->setChecked(checked);
}

void MainWindow::on_actionRender_multi_res_mesh_changed()
{
    if(ui->actionRender_multi_res_mesh->isChecked())
    {
        scene_->multiResFirstStepMesh();
        ui->actionMesh_Line_tool->setEnabled(false);
        ui->actionMesh_Collapse_Edge->setEnabled(false);
        ui->actionMesh_Insert_on_Edge->setEnabled(false);
        ui->actionMesh_Mesh_Insertion->setEnabled(false);
        ui->actionMesh_Rectangle_Tool->setEnabled(false);
        ui->actionMesh_move_and_select->trigger();
        ui->renderSpinBoxMultiRes->setEnabled(false);
    }
    else
    {
        QString message("This will delete changes made on the multi res mesh. Are you sure?");
        QMessageBox::StandardButton response = QMessageBox::question(nullptr,"Multi resh mesh",message);
        if(response == QMessageBox::Yes)
        {
            scene_->resetMultiResMesh();
            ui->actionMesh_Line_tool->setEnabled(true);
            ui->actionMesh_Collapse_Edge->setEnabled(true);
            ui->actionMesh_Insert_on_Edge->setEnabled(true);
            ui->actionMesh_Mesh_Insertion->setEnabled(true);
            ui->actionMesh_Rectangle_Tool->setEnabled(true);
            scene_->setRenderVertsEdges(true);
            ui->renderSpinBoxMultiRes->setEnabled(true);
        }
    }
    scene_->update();
}

void MainWindow::on_actionMesh_Line_tool_triggered()
{
    scene_->setDrawingMode(GMView::drawModeCanvas::lineTool);
}

void MainWindow::on_actionMesh_move_and_select_triggered()
{
    scene_->setDrawingMode(GMView::drawModeCanvas::move);
}

void MainWindow::on_actionMesh_Insert_on_Edge_triggered()
{
    scene_->setDrawingMode(GMView::drawModeCanvas::insertVert);
}

void MainWindow::on_actionMesh_Collapse_Edge_triggered()
{
    scene_->setDrawingMode(GMView::drawModeCanvas::collapseEdge);
}

void MainWindow::on_actionMesh_Mesh_Insertion_triggered()
{
    scene_->setDrawingMode(GMView::drawModeCanvas::meshToolInsertion);
}

void MainWindow::on_actionMesh_Rectangle_Tool_triggered()
{
    scene_->setDrawingMode(GMView::drawModeCanvas::rectangleTool);
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
        QColor prevColor = palette.color(QPalette::Button);
        if(prevColor == Qt::black) prevColor = Qt::white;
        QColor chosenColor = QColorDialog::getColor(prevColor);
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
    scene_->resetToBeFaceVector();
    scene_->clearSelection();
}

void MainWindow::on_actionHelpAbout_Qt_triggered()
{
     QApplication::aboutQt();
}

void MainWindow::on_imageImport_clicked()
{
    QFileDialog dialog(this, "Import image");

    //Code below from: http://doc.qt.io/qt-5/qtwidgets-widgets-imageviewer-example.html
    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = QImageReader::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    mimeTypeFilters.append("application/octet-stream");
    dialog.setMimeTypeFilters(mimeTypeFilters);
    //dialog.selectMimeTypeFilter("image/jpeg");

    if(dialog.exec() == QDialog::Accepted)
    {
        QString filename = dialog.selectedFiles().first();
        qDebug() << filename;
        scene_->handleImageFileDialog(filename, true);
    }
}

void MainWindow::on_imageRemove_clicked()
{
    scene_->handleImageFileDialog("", false);
}

void MainWindow::on_imageOpacity_valueChanged(int value)
{
    if(scene_->imageItem() != nullptr) scene_->imageItem()->setOpacity((double)value/100);
}

void MainWindow::on_action_New_triggered()
{
   scene_->clear();
   layerModel_->clear();
   vector<GMView::CanvasItemGroup *> canvasLayers = scene_->layers();
   for(GMView::CanvasItemGroup *item : canvasLayers)
   {
       layerModel_->appendRow(item);
   }
}

void MainWindow::on_renderSpinBox_valueChanged(int value)
{
    GUILogic::MeshHandler::setSubdivisionSteps(value);
}

void MainWindow::on_renderSpinBoxMultiRes_valueChanged(int value)
{
    if(value > ui->renderSpinBox->value()) ui->renderSpinBoxMultiRes->setValue(value-1);
    GUILogic::MeshHandler::setMultiResSubdivisionSteps(value);
}


void MainWindow::on_renderRenderButton_clicked()
{
    scene_->prepareRendering();
}

void MainWindow::on_renderAutoRenderCheckBox_clicked(bool checked)
{
    ui->actionRender_auto->setChecked(checked);
}

void MainWindow::on_actionRender_GUI_faces_triggered(bool checked)
{
    scene_->setRenderFace(checked);
    scene_->update();
}

void MainWindow::on_imageColorFrom_clicked()
{
    scene_->setItemPointColorFromImage();
}

void MainWindow::on_glRenderButton_fullScene_clicked()
{
    ui->openGLWidget->setRenderMode(GMView::renderModeOpenGL::fullSceneRender);
}

void MainWindow::on_glRenderButton_viewport_clicked()
{
    ui->openGLWidget->setRenderMode(GMView::renderModeOpenGL::viewportRender);
}

void MainWindow::on_glRenderButton_boundingRect_clicked()
{
    ui->openGLWidget->setRenderMode(GMView::renderModeOpenGL::boundingRect);
}
