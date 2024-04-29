#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ModelPartList.h"
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void updateRender();
    void updateRenderFromTree(const QModelIndex& index);
    void resetCamera();
    void loadStlFile(const QString& fileName);
public slots:
    ModelPart* settingsDialog();
    void buttonNotInUse();
    void handleTreeClicked();
signals:
    void statusUpdateMessage(const QString & message, int timeout);
private slots:
    void on_actionOpen_File_triggered();
    void on_actionItem_Options_triggered();

private:
    Ui::MainWindow *ui;
    ModelPartList* partList;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;


};
#endif // MAINWINDOW_H
