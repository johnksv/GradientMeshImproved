#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void on_actionVertices_only_triggered();

    void on_actionFull_render_Change_name_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
