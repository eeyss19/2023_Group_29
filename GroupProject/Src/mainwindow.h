#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ModelPartList.h"
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include "VRRenderThread.h"

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
    void VRActorsFromTree(const QModelIndex& index);
    void resetCamera();
    void updateVRthread();
    void loadStlFile(const QString& fileName);
    void update_name();
    
public slots:
    void settingsDialog();
    void vrButton();
    void handleTreeClicked();
    void startVR();
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
    VRRenderThread* vrThread = nullptr;

};
#endif // MAINWINDOW_H
