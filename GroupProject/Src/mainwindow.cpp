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

    connect(ui->pushButton, &QPushButton::released , this, &MainWindow::settingsDialog);
    connect(ui->pushButton_2, &QPushButton::released, this, &MainWindow::vrButton);
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::handleTreeClicked);
    connect(this, &MainWindow::statusUpdateMessage, ui->statusbar, &QStatusBar::showMessage);

    vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
    light->SetLightTypeToSceneLight();
    light->SetPosition(0, 0, 0);
    light->SetPositional(true);
    light->SetConeAngle(10);
    light->SetFocalPoint(0, 0, 0);
    light->SetIntensity(0.5); // Reduce the intensity
    light->SetDiffuseColor(0.8, 0.8, 0.8); // Make the light less bright
    light->SetAmbientColor(0.2, 0.2, 0.2); // Reduce the ambient light
    light->SetSpecularColor(0.8, 0.8, 0.8); // Reduce the specular light

    
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
    renderer->SetBackground(colors->GetColor3d("black").GetData()); // Set background color to 
    //renderer->AddLight(light);                                    // Doesn't working, ModelParts appear black and not visible
    renderWindow->AddRenderer(renderer);
    
    renderWindow->AddRenderer(renderer);
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
    //vrThread->stop();
}


/**
 * @brief Handles the second button click event.
 */
void MainWindow::vrButton(){
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
    ModelPart* newItem = new ModelPart({ fileName, selectedPart->get_Visibility() });
    newItem->setName(fileName);
    selectedPart->appendChild(newItem);

    // Call the loadSTL() function of the newly created item to ask it to load from the STL file.
    newItem->loadSTL(fileName);
}

void MainWindow::update_name()
{
	QModelIndex index = ui->treeView->currentIndex();
	ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    selectedPart->set(0, selectedPart->get_Name());
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
        if (!selectedPart->get_Visibility()) {
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
        QColor color = selectedPart->get_Color(); // Assuming there is a get_Color() method in ModelPart class
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

void MainWindow::startVR()
{
    VRActorsFromTree(partList->index(0, 0, QModelIndex()));
    vrThread->start();
}


void MainWindow::VRActorsFromTree(const QModelIndex& index)
{
    if (index.isValid()) {
        ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
        selectedPart->set(1, "true");

        // Check if the ModelPart is visible
        if (!selectedPart->get_Visibility()) {
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
        QColor color = selectedPart->get_Color(); // Assuming there is a get_Color() method in ModelPart class
        VRactor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
        vrThread->addActorOffline(VRactor);
    }

    resetCamera();
    
}

/**
 * @brief Resets the camera position.
 */
void MainWindow::resetCamera()
{
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Azimuth(150);
    renderer->GetActiveCamera()->Elevation(30);
    renderer->ResetCameraClippingRange();
}

void MainWindow::updateVRthread()
{
    vrThread->stop();
    // Check that the vrThread is initialized
    if (vrThread == nullptr) {
		qDebug() << "VR Thread not initialized";
		return;
	}
	// Remove all actors from vrThread
    vrThread->removeAllActors();
    VRActorsFromTree(partList->index(0, 0, QModelIndex()));
    vrThread->start();
}
// 
// Open file creates a top level with the name of the 
// Rotation in 
// Add part at runtime
// Stop 
// VTK Actor has functional visibility but not the VR actor?
// 
// Demonstrate two filters working.These can be applied to parts of the model independently (e.g.only to
// a wheel), and can be applied in any combination.
//
// you can change things in the GUI and the effect is seen in VR, while it is running.E.g.changing colour,
// visible status, add an extra STL file, etc.
// 
// Interaction with model using VR controllers.The ideal case is that every single sub assembly from the
// Level2 model can be manipulated independently, but this may not be feasible.How interactive can you
// make the experience ? This could be using code, or by additional partitioning of the CAD model, or both.
// 
// Add some animation : e.g.the rotation hinted at in the renderThread class, or something more advanced 
// Virtual environment - can you add a floor, scenery, etc either manually or with a Skybox
// #file:'mainwindow.cpp' #file:'mainwindow.h' #file:'ModelPart.cpp' #file:'ModelPart.h' #file:'ModelPartList.cpp' #file:'ModelPartList.h' #file:'VRRenderThread.cpp' #file:'VRRenderThread.h' #file:'optiondialog.cpp' #file:'optiondialog.h' 

