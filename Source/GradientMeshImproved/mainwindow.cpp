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
#include <QStringList>

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

    ui->graphicsView->setFocus();
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
    ui->imageWidget->setHidden(true);
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
    QStandardItemModel *layerModel = new QStandardItemModel;
    scene_->setLayerModel(layerModel);
    ui->layer_listView->setModel(layerModel);
    ui->layer_listView->setSelectionBehavior(QAbstractItemView::SelectRows);
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

void MainWindow::on_actionSave_layer_triggered()
{
    //For PNG filter use OFF file (*.off);;PNG (*.png)
    QString filename = QFileDialog::getSaveFileName(this,tr("Export as file"),
                                                    "","OFF file (*.off)");

    QString format = filename.split(".").back();
    if ( QString::compare(format, "off", Qt::CaseInsensitive) == 0)
    {
        scene_->saveLayerToOFF(filename);
    }
}

void MainWindow::on_actionSave_all_layers_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,tr("Export as file"),
                                                    "","OFF file (*.off)");
    if(filename == "") return;
    scene_->saveAllLayersToOFF(filename);
}

void MainWindow::on_actionImport_triggered()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this,tr("Import file"),
                                                    "", tr("OFF file (*.off)"));
    if(filenames.isEmpty()) return;
    bool sucsess = false;
    if(filenames.size() == 1){
        sucsess = scene_->importFile(filenames.first());

    }else{
        sucsess = scene_->importFile(filenames.first());
        if(!sucsess) return;

        for(int i =1; i < filenames.size(); i++){
            scene_->importFileLayer(filenames.at(i));
        }
    }


    if(sucsess){
        QString msg("File imported! Hit 'Render mesh' to subdivied mesh");
        QMessageBox::information(nullptr, "Import file", msg);
    }
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
        //TODO: feedback that it is hidden/visible
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
    if(! ui->layer_listView->selectionModel()->selectedIndexes().isEmpty())
    {
                if(ui->layer_listView->model()->rowCount() == 1 )
                {
                    QMessageBox msgBox;
                    msgBox.setText("Can not delete base layer");
                    msgBox.exec();
                }
                else
                {
                    int currentIndex  = ui->layer_listView->selectionModel()->currentIndex().row();
                    scene_->deleteLayer(currentIndex);
                    layerMoveButtonsUpdate(currentIndex);
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
    scene_->addLayer();
    scene_->setActiveLayerBack();
    int currentIndex  = ui->layer_listView->selectionModel()->currentIndex().row();
    layerMoveButtonsUpdate(currentIndex);

}

void MainWindow::on_layer_listView_clicked(const QModelIndex &index)
{
    scene_->setActiveLayer(index.row());
    //Update move up and down layer
    layerMoveButtonsUpdate(index.row());
}

void MainWindow::on_layerMoveUp_clicked()
{
    if(! ui->layer_listView->selectionModel()->selectedIndexes().isEmpty())
    {
        QModelIndex index  = ui->layer_listView->selectionModel()->currentIndex();
        scene_->moveLayerUp(index.row());
        ui->layer_listView->setCurrentIndex(index);
        scene_->setActiveLayer(index.row());
    }
    else
    {
        QMessageBox msgBox;
            msgBox.setText("No layers are selected. ");
            msgBox.exec();
    }
}

void MainWindow::on_layerMoveDown_clicked()
{
    if(! ui->layer_listView->selectionModel()->selectedIndexes().isEmpty())
    {
        QModelIndex index  = ui->layer_listView->selectionModel()->currentIndex();
        scene_->moveLayerDown(index.row());
        ui->layer_listView->setCurrentIndex(index);
    }
    else
    {
        QMessageBox msgBox;
            msgBox.setText("No layers are selected. ");
            msgBox.exec();
    }
}

void MainWindow::on_layer_listView_activated(const QModelIndex &index)
{
    layerMoveButtonsUpdate(index.row());
}

void MainWindow::layerMoveButtonsUpdate(const int index)
{
    bool enableDown = true;
    bool enableUp = true;
    int rowCount = ui->layer_listView->model()->rowCount();
    if(rowCount == 1){
        enableDown = enableUp = false;
    }else if(index <= 0){
        enableUp = false;
    }else if(index + 1 >= rowCount){
        enableDown = false;
    }
    ui->layerMoveDown->setEnabled(enableDown);
    ui->layerMoveUp->setEnabled(enableUp);
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
   scene_->newDocument();
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
