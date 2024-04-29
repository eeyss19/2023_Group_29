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
    connect(ui->pushButton_2, &QPushButton::released, this, &MainWindow::buttonNotInUse);
    connect(this, &MainWindow::statusUpdateMessage, ui->statusbar, &QStatusBar::showMessage);
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::handleTreeClicked);
    

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
ModelPart* MainWindow::settingsDialog(){
    QModelIndex index = ui->treeView->currentIndex();

    /* Get a pointer to the item from the index */
    ModelPart *selectedPart = static_cast<ModelPart*>(index.internalPointer());

    emit statusUpdateMessage(QString("Right button clicked!"), 0);
    OptionDialog dialog(this);
    dialog.set_ptr(selectedPart);
    dialog.loadSettings();

    connect(&dialog, &OptionDialog::settingsSaved, this, &MainWindow::updateRender);

    if (dialog.exec() == QDialog::Accepted) {
        emit statusUpdateMessage(QString("Dialog accepted"), 0);
    } else {
        emit statusUpdateMessage(QString("Dialog rejected"), 0);
    }
    return selectedPart;
}

/**
 * @brief Handles the second button click event.
 */
void MainWindow::buttonNotInUse(){
    /*ModelPart* parent = this->partList->getRootItem()->parentItem();
    int NoChild = parent->childCount();
    for(int i = 0 ; i < NoChild ; ++i)
        delete parent->child(i);

    ui->treeView->reset();*/
    //this->partList->getRootItem()->deleteChildren();
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
void MainWindow::on_actionOpen_File_triggered()
{
    qDebug() << "on_actionOpen_File_triggered() called"; // Debug output

    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        tr("Open Files or Directories"),
        "C:\\",
        tr("STL Files (*.stl);;Directories (*)")
    );

    foreach(QString path, fileNames) {
        QFileInfo fileInfo(path);

        if (fileInfo.isFile()) {
            // Handle file
            loadStlFile(path);
        }
        else if (fileInfo.isDir()) {
            // Handle directory
            QDir dir(path);
            QFileInfoList stlFiles = dir.entryInfoList(QStringList() << "*.stl", QDir::Files, QDir::DirsFirst);

            foreach(QFileInfo stlFileInfo, stlFiles) {
                loadStlFile(stlFileInfo.absoluteFilePath());
            }
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
    selectedPart->appendChild(newItem);

    // Call the loadSTL() function of the newly created item to ask it to load from the STL file.
    newItem->loadSTL(fileName);
}



/**
 * @brief Handles the "Item Options" action trigger.
 */
void MainWindow::on_actionItem_Options_triggered(){
    // Add some sort of dropdown?
}

/**
 * @brief Updates the render window.
 */
void MainWindow::updateRender()
{
    renderer->RemoveAllViewProps();
    updateRenderFromTree(partList->index(0, 0, QModelIndex()));
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
        vtkActor* actor = selectedPart->getActor(); // Assuming there is a getActor() method in ModelPart class

        // Check if the actor is not null
        if (actor == nullptr) {
            qDebug() << "Failed to get actor from model part";
            return;
        }

        // Get the color from the ModelPart and set it to the actor
        QColor color = selectedPart->get_Color(); // Assuming there is a get_Color() method in ModelPart class
        actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());

        // Check if the actor is visible
        if (!actor->GetVisibility()) {
            qDebug() << "Actor is not visible";
            return;
        }

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
void MainWindow::resetCamera()
{
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Azimuth(150);
    renderer->GetActiveCamera()->Elevation(30);
    renderer->ResetCameraClippingRange();
}

