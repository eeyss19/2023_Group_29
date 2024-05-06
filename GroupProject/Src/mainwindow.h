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
    void loadStlFile(const QString& fileName);  
    void update_name();
    
public slots:
    void settingsDialog();
    void handleTreeClicked();
    void startVR();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();

signals:
    void statusUpdateMessage(const QString & message, int timeout);

private slots:
    void on_actionOpen_File_triggered();
    void on_actionItem_Options_triggered();
    void on_actionSave_Screenshot_triggered();
    void on_actionStart_VR_triggered();
    void on_actionStop_VR_triggered();
    void on_actionChange_Background_triggered();
    void on_actionChange_App_Color_triggered();
    void on_actionHow_to_Use_triggered();
    void on_actionClip_Filter_triggered();
    void on_actionShrink_Filter_triggered();
    void on_actionEdit_Properties_triggered();

private:
    Ui::MainWindow *ui;
    ModelPartList* partList;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    VRRenderThread* vrThread = nullptr;

};
#endif // MAINWINDOW_H
