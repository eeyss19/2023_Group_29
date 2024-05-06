#include "VRRenderThread.h"
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QStatusBar>
#include <QFileDialog>
#include "ModelPart.h"
#include "optiondialog.h"
#include <QDebug>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkNamedColors.h>
#include <QPixMap>
#include <qmessagebox.h>
#include <vtkLight.h>
// Other includes come after

/**
 * @brief MainWindow constructor.
 * @param parent The parent widget.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->treeView->addAction(ui->actionItem_Options);
    disconnect(ui->pushButton_5, 0, this, 0);
    disconnect(ui->pushButton_6, 0, this, 0);

    // Now connect the QPushButton::clicked signal to the on_pushButton_5_clicked() slot
    connect(ui->pushButton_5, &QPushButton::clicked, this, &MainWindow::on_pushButton_5_clicked);
    connect(ui->pushButton_6, &QPushButton::clicked, this, &MainWindow::on_pushButton_6_clicked);

    connect(ui->pushButton, &QPushButton::released , this, &MainWindow::settingsDialog);
    connect(ui->pushButton_2, &QPushButton::released, this, &MainWindow::on_pushButton_2_clicked);
    connect(ui->pushButton_3, &QPushButton::released, this, &MainWindow::on_pushButton_3_clicked);
    connect(ui->pushButton_4, &QPushButton::released, this, &MainWindow::on_pushButton_4_clicked);
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::handleTreeClicked);
    connect(this, &MainWindow::statusUpdateMessage, ui->statusbar, &QStatusBar::showMessage);
    
    

    //Button Colors
    ui->pushButton->setStyleSheet("background-color: black; color: white; font-size: 12px; font-weight: bold;");
    ui->pushButton_2->setStyleSheet("background-color: black; color: white; font-size: 12px; font-weight: bold;");
    ui->pushButton_3->setStyleSheet("background-color: green; color: white; font-size: 14px; font-weight: bold;");
    ui->pushButton_4->setStyleSheet("background-color: red; color: white; font-size: 14px; font-weight: bold;");
    ui->pushButton_5->setStyleSheet("background-color: purple; color: white; font-size: 12px; font-weight: bold;");
    ui->pushButton_6->setStyleSheet("background-color: purple; color: white; font-size: 12px; font-weight: bold;");

    /* Create / allocate the ModelList */
    this->partList = new ModelPartList("PartsList");

    /* Link it to the treeview in the GUI */
    ui->treeView->setModel(this->partList);

    /* Manually create a model tree - there are much better and more flexible ways of doing this,
    e.g. with nested functions. This is just a quick example as a starting point. */
    ModelPart *rootItem = this->partList->getRootItem();

    /* Add 3 top level items */
    for (int i = 0; i < 3; i++) {
        /* Create strings for both data columns */
        QString name = QString("TopLevel %1").arg(i);
        QString visible("true");

        /* Create child item */
        ModelPart *childItem = new ModelPart({name, visible});
        childItem->setName(name);
        childItem->setTopLevelBool(true);

        /* Append to tree top-level */
        rootItem->appendChild(childItem);
    }

    /* This needs adding to MainWindow constructor */
    /* Link a render window with the Qt widget */
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->widget->setRenderWindow(renderWindow);

    /* Add a renderer */
    vtkNew<vtkNamedColors> colors;
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(colors->GetColor3d("white").GetData()); // Set background color to white
    renderWindow->AddRenderer(renderer);

    /* Add a light */
    vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
    light->SetLightTypeToSceneLight();
    light->SetPosition(1.0, 1.0, 1.0);
    renderer->AddLight(light);

    resetCamera();
}

/**
 * @brief MainWindow destructor.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Handles the button click event.
 * @return The selected ModelPart.
 */
void MainWindow::settingsDialog(){
    QModelIndex index = ui->treeView->currentIndex();

    /* Get a pointer to the item from the index */
    ModelPart *selectedPart = static_cast<ModelPart*>(index.internalPointer());

    OptionDialog dialog(this);
    dialog.set_ptr(selectedPart);
    dialog.loadSettings();

    if (dialog.exec() == QDialog::Accepted) {
        emit statusUpdateMessage(QString("Dialog accepted"), 0);
    } else {
        emit statusUpdateMessage(QString("Dialog rejected"), 0);
    }
}

/**
 * @brief Handles the "Item Options" action trigger.
 */
void MainWindow::on_actionItem_Options_triggered(){
    settingsDialog();
}

/**
 * @brief Handles the second button click event.
 */
void MainWindow::on_pushButton_2_clicked()
{
    QModelIndex index = ui->treeView->currentIndex();
    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (selectedPart) {
        // Reset the position of the selected part to its original position
        selectedPart->resetToOriginalPosition();
        updateRender(); // Update the render window to reflect the changes
        resetCamera(); // Reset the camera
    }
}
/**
 * @brief Handles the second button click event.
 */
void MainWindow::on_pushButton_3_clicked(){
    vrThread = new VRRenderThread();
    startVR();
}

/**
 * @brief Handles the tree view click event.
 */
void MainWindow::handleTreeClicked(){
    /* Get the index of the selected item */
    QModelIndex index = ui->treeView->currentIndex();

    /* Get a pointer to the item from the index */
    ModelPart *selectedPart = static_cast<ModelPart*>(index.internalPointer());

    /* In this case, we will retrieve the name string from the internal QVariant data array */
    QString text = selectedPart->data(0).toString();

    emit statusUpdateMessage(QString("The selected item is: ") + text, 0);
}


/**
 * @brief Handles the "Open File" action trigger.
 */
void MainWindow::on_actionOpen_File_triggered(){

    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        tr("Open Files"),
        "C:\\",
        tr("STL Files (*.stl)")
    );

    foreach(QString path, fileNames) {
        QFileInfo fileInfo(path);

        if (fileInfo.isFile()) {
            // Handle file
            loadStlFile(path);
        }
    }
    updateRender();
}

void MainWindow::loadStlFile(const QString& fileName)
{
    emit statusUpdateMessage(QString("The selected file is: ") + fileName, 0);

    // Use the fileName to open a new child item in the tree
    QModelIndex index = ui->treeView->currentIndex();
    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    ModelPart* newItem = new ModelPart({ fileName, selectedPart->getVisibility() });
    newItem->setName(fileName);
    selectedPart->appendChild(newItem);

    // Call the loadSTL() function of the newly created item to ask it to load from the STL file.
    newItem->loadSTL(fileName);
    resetCamera();
    updateRender();
}

void MainWindow::update_name()
{
	QModelIndex index = ui->treeView->currentIndex();
	ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
	selectedPart->set(0, selectedPart->getName());
	updateRender();
}

/**
 * @brief Updates the render window.
 */
void MainWindow::updateRender()
{
    renderer->RemoveAllViewProps();
    updateRenderFromTree(partList->index(0, 0, QModelIndex())); // Ask about this function and index as well
    renderer->Render();
    renderWindow->Render();
}


/**
 * @brief Updates the render window from the model tree.
 * @param index The index of the model tree.
 */
void MainWindow::updateRenderFromTree(const QModelIndex& index)
{
    if (index.isValid()) {
        ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
        selectedPart->set(1, "true");

        // Check if the ModelPart is visible
        if (!selectedPart->getVisibility()) {
            selectedPart->set(1, "false"); // Assuming there is a set() method in ModelPart class
            return;
        }
        
        vtkActor* actor = selectedPart->getActor(); // Assuming there is a getActor() method in ModelPart class

        // Check if the actor is not null
        if (actor == nullptr) {
            qDebug() << "Failed to get actor from model part";
            return;
        }

        // Get the color from the ModelPart and set it to the actor
        QColor color = selectedPart->getColor(); // Assuming there is a get_Color() method in ModelPart class
        actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());

        renderer->AddActor(actor);
    }

    // Check to see if this part has any children
    if (!partList->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren)) {
        return;
    }

    // Loop through children and add their actors
    int rows = partList->rowCount(index);
    for (int i = 0; i < rows; i++) {
        updateRenderFromTree(partList->index(i, 0, index));
    }
    resetCamera();
}

/**
 * @brief Resets the camera position.
 */
void MainWindow::resetCamera() {
    renderer->ResetCamera(); // Adjust as needed to fit your scene
    renderWindow->Render();
}

void MainWindow::startVR()
{
    if (!vrThread || vrThread->isRunning()) {
		return; // VR is already running or no instance available
	}
    VRActorsFromTree(partList->index(0, 0, QModelIndex()));
    vrThread->start();
}

void MainWindow::on_pushButton_4_clicked() {
 
    
    if (vrThread && vrThread->isRunning()) {
        vrThread->issueCommand(VRRenderThread::END_RENDER, 0);
        vrThread->wait(); // Wait for the thread to finish
        delete vrThread; // Clean up
        vrThread = nullptr; // Reset pointer
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    QPixmap originalPixmap = this->grab();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Screenshot"),
        QDir::currentPath(),
        tr("Images (*.png *.xpm *.jpg)"));
    if (!fileName.isEmpty())
        originalPixmap.save(fileName);

}


void MainWindow::on_pushButton_6_clicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Choose Background Color");
    if (color.isValid()) {
        double r = color.redF();
        double g = color.greenF();
        double b = color.blueF();
        renderer->SetBackground(r, g, b);
        renderWindow->Render();
    }
}

void MainWindow::on_actionShrink_Filter_triggered()
{
    const bool filterFlag = ui->actionShrink_Filter->isChecked();
    QModelIndex index = ui->treeView->currentIndex();
    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    selectedPart->shrink(filterFlag);
    updateRender();
    renderWindow->Render();
    resetCamera();
}


void MainWindow::on_actionSave_Screenshot_triggered()
{
    on_pushButton_5_clicked();
}


void MainWindow::on_actionStart_VR_triggered()
{
    on_pushButton_3_clicked();

}


void MainWindow::on_actionStop_VR_triggered()
{
    on_pushButton_4_clicked();

}


void MainWindow::on_actionChange_Background_triggered()
{
    on_pushButton_6_clicked();

}


void MainWindow::on_actionChange_App_Color_triggered()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color");
    if (color.isValid()) {
        QString colorStyle = QString("background-color: %1").arg(color.name());
        this->setStyleSheet(colorStyle);

        // Set a specific style for the buttons and text
        ui->pushButton->setStyleSheet("background-color: none; color: black; font-size: 14px; font-weight: bold;");
        ui->pushButton_2->setStyleSheet("background-color: none; color: black; font-size: 14px; font-weight: bold;");
        ui->pushButton_3->setStyleSheet("background-color: none; color: black; font-size: 14px; font-weight: bold;");
        ui->pushButton_4->setStyleSheet("background-color: none; color: black; font-size: 14px; font-weight: bold;");
        ui->pushButton_5->setStyleSheet("background-color: none; color: black; font-size: 14px; font-weight: bold;");
        ui->pushButton_5->setStyleSheet("background-color: none; color: black; font-size: 14px; font-weight: bold;");
        ui->pushButton_6->setStyleSheet("background-color: none; color: black; font-size: 14px; font-weight: bold;");
    }
}


void MainWindow::on_actionHow_to_Use_triggered()
{
    QMessageBox::information(this, tr("How to Use"),
        tr("Here are the instructions on how to use the app:\n\n"
            "1. Load STL File: ...\n"
            "2. Reset View: ...\n"
            "3. Open VR: ...\n"
            "4. Close VR: ...\n"
            "5. Take Screenshot: ...\n"
            "6. Change Background Color: ..."));
}

void MainWindow::on_actionClip_Filter_triggered()
{
	const bool filterFlag = ui->actionClip_Filter->isChecked();
	QModelIndex index = ui->treeView->currentIndex();
	ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
	selectedPart->clip(filterFlag);
	updateRender();
	renderWindow->Render();
	resetCamera();
}


void MainWindow::on_actionEdit_Properties_triggered()
{
    settingsDialog();
}


void MainWindow::VRActorsFromTree(const QModelIndex& index)
{
    if (index.isValid()) {
        ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
        selectedPart->set(1, "true");

        // Check if the ModelPart is visible
        if (!selectedPart->getVisibility()) {
            selectedPart->set(1, "false"); // Assuming there is a set() method in ModelPart class
            return;
        }

        vtkActor* VRactor = selectedPart->getNewActor(); // Assuming there is a getActor() method in ModelPart class

        // Check if the actor is not null
        if (VRactor == nullptr) {
            if (!partList->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren)) {
                return;
            }

            // Loop through children and add their actors
            int rows = partList->rowCount(index);
            for (int i = 0; i < rows; i++) {
                VRActorsFromTree(partList->index(i, 0, index));
            }
            return;
        }

        // Get the color from the ModelPart and set it to the actor
        QColor color = selectedPart->getColor(); // Assuming there is a get_Color() method in ModelPart class
        VRactor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
        vrThread->addActorOffline(VRactor);
    }

    resetCamera();
    
}

// Stop 
// 
// VTK Actor has functional visibility but not the VR actor?

// Demonstrate two filters working.These can be applied to parts of the model independently (e.g.only to
// a wheel), and can be applied in any combination.

// Add some animation : e.g.the rotation hinted at in the renderThread class, or something more advanced 
// Virtual environment - can you add a floor, scenery, etc either manually or with a Skybox
// #file:'mainwindow.cpp' #file:'mainwindow.h' #file:'ModelPart.cpp' #file:'ModelPart.h' #file:'ModelPartList.cpp' #file:'ModelPartList.h' #file:'VRRenderThread.cpp' #file:'VRRenderThread.h' #file:'optiondialog.cpp' #file:'optiondialog.h' 

