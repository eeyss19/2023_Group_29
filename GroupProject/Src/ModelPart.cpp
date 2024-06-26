/**     @file ModelPart.cpp
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Template for model parts that will be added as treeview items
  *
  *     P Evans 2022
  */

#include "ModelPart.h"


/* Commented out for now, will be uncommented later when you have
 * installed the VTK library
 */
#include <vtkDataSetMapper.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <QVector3D>
#include <vtkShrinkFilter.h>
#include <vtkGeometryFilter.h>
#include <vtkPlane.h>
#include <vtkClipDataSet.h>


ModelPart::ModelPart(const QList<QVariant>& data, ModelPart* parent)
    : m_itemData(data), m_parentItem(parent), originalPosition(QVector3D(0, 0, 0)) {
    file = nullptr;
    mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    actor = vtkSmartPointer<vtkActor>::New();
    /* You probably want to give the item a default colour - Initalized default with colour */
    actor->VisibilityOn();      // Do the same with this, ModelPart should be default visible.
}

/**
 * @brief Destructor for the ModelPart class.
 *
 * Deletes all child items of the current model part.
 */
ModelPart::~ModelPart() {
    qDeleteAll(m_childItems);
    
}

void ModelPart::appendChild( ModelPart* item ) {
    /* Add another model part as a child of this part
     * (it will appear as a sub-branch in the treeview)
     */
    item->m_parentItem = this;
    m_childItems.append(item);
}

ModelPart* ModelPart::child( int row ) {
    /* Return pointer to child item in row below this item.
     */
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int ModelPart::childCount() const {
    /* Count number of child items
     */
    return m_childItems.count();
}

int ModelPart::columnCount() const {
    /* Count number of columns (properties) that this item has.
     */
    return m_itemData.count();
}

QVariant ModelPart::data(int column) const {
    /* Return the data associated with a column of this item 
     *  Note on the QVariant type - it is a generic placeholder type
     *  that can take on the type of most Qt classes. It allows each 
     *  column or property to store data of an arbitrary type.
     */
    if (column < 0 || column >= m_itemData.size())
        return QVariant();
    return m_itemData.at(column);
}

void ModelPart::set(int column, const QVariant &value) {
    /* Set the data associated with a column of this item 
     */
    if (column < 0 || column >= m_itemData.size())
        return;

    m_itemData.replace(column, value);
}

ModelPart* ModelPart::parentItem() {
    return m_parentItem;
}

int ModelPart::row() const {
    /* Return the row index of this item, relative to it's parent.
     */
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ModelPart*>(this));
    return 0;
}
/**
 * @brief Loads an STL file for the model part.
 *
 * @param fileName The name of the STL file to load.
 */
void ModelPart::loadSTL(QString fileName) {
    // Load the STL file
    file = vtkSmartPointer<vtkSTLReader>::New();
    file->SetFileName(fileName.toStdString().c_str());
    file->Update();

    // Check if the file is loaded correctly
    if (file->GetOutput() == nullptr) {
        qDebug() << "Failed to load STL file: " << fileName;
        return;
    }

    // Initialize the part's mapper and actor
    mapper->SetInputConnection(file->GetOutputPort());
    actor->SetMapper(mapper);

    // Set the original position now that the STL is loaded
    if (originalPosition == QVector3D(0, 0, 0)) {
        originalPosition = QVector3D(actor->GetPosition()[0], actor->GetPosition()[1], actor->GetPosition()[2]);
    }

    // Update the VTK actor's position
    actor->SetPosition(originalPosition.x(), originalPosition.y(), originalPosition.z());
}


/**
 * @brief Sets the color of the model part.
 *
 * @param Clr The color to set for the model part.
 */
void ModelPart::setColour(QColor Clr) {
    if ((Clr != Colour) && (this->getTopLevelBool())){
        Colour = Clr;
        // Set the colour of all children to the same colour
        for (int i = 0; i < m_childItems.size(); i++) {
			m_childItems[i]->setColour(Clr);
		}
        return;
	}
    Colour = Clr;
}

/**
 * @brief Sets the visibility of the model part.
 *
 * @param isvisible The visibility status to set for the model part.
 */

void ModelPart::setVisible(const bool isvisible) {
    isVisible = isvisible;
}

/**
 * @brief Sets the name of the model part.
 *
 * @param name The name to set for the model part.
 */
void ModelPart::setName(const QString name) {
    Name = name;
}

/**
 * @brief Gets the color of the model part.
 *
 * @return The color of the model part.
 */
const QColor ModelPart::getColor(void) {
    return Colour;
}

void ModelPart::shrink(const bool filterFlag) {
    if (this->getTopLevelBool()) {
        for (int i = 0; i < m_childItems.size(); i++) {
            m_childItems[i]->shrink(filterFlag);
        }
        return;
    }
    shrinkStatus = filterFlag;
    applyFilters();
}

void ModelPart::clip(const bool filterFlag) {
    if (this->getTopLevelBool()) {
        for (int i = 0; i < m_childItems.size(); i++) {
            m_childItems[i]->clip(filterFlag);
        }
        return;
    }
    clipStatus = filterFlag;
    applyFilters();
}

void ModelPart::applyFilters() {
    vtkSmartPointer<vtkAlgorithm> lastFilter = file;

    if (shrinkStatus) {
        vtkSmartPointer<vtkShrinkFilter> shrinkFilter = vtkSmartPointer<vtkShrinkFilter>::New();
        shrinkFilter->SetInputConnection(lastFilter->GetOutputPort());
        shrinkFilter->SetShrinkFactor(0.5);
        shrinkFilter->Update();
        lastFilter = shrinkFilter;
    }

    if (clipStatus) {
        vtkSmartPointer<vtkPlane> planeLeft = vtkSmartPointer<vtkPlane>::New();
        planeLeft->SetOrigin(0.0, 0.0, 0.0);
        planeLeft->SetNormal(0.0, 1.0, 0.0);

        vtkSmartPointer<vtkClipDataSet> clipFilter = vtkSmartPointer<vtkClipDataSet>::New();
        clipFilter->SetInputConnection(lastFilter->GetOutputPort());
        clipFilter->SetClipFunction(planeLeft.Get());
        clipFilter->Update();
        lastFilter = clipFilter;
    }

    vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
    geometryFilter->SetInputConnection(lastFilter->GetOutputPort());
    geometryFilter->Update();

    mapper->SetInputConnection(geometryFilter->GetOutputPort());
    actor->SetMapper(mapper);
}
/**
 * @brief Gets the name of the model part.
 *
 * @return The name of the model part.
 */
const QString ModelPart::getName(void) {
    return Name;
}

/**
 * @brief Gets the visibility status of the model part.
 *
 * @return The visibility status of the model part.
 */
const bool ModelPart::getVisibility(void) {
    return isVisible;
}

/**
 * @brief Gets the actor of the model part.
 *
 * @return The actor of the model part.
 */
const vtkSmartPointer<vtkActor> ModelPart::getActor() {
    return actor;
}

vtkActor* ModelPart::getNewActor() {

    if (file == nullptr) {
        qDebug() << "ERROR: nothing in file reader";
        return nullptr;
    }                                                   // NULL check before assignment, potential fix?
    pd = vtkSmartPointer<vtkPolyData>::New();
    pd->DeepCopy(mapper->GetInputDataObject(0, 0));
    /* 1. Create new mapper */
    vrMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    vrMapper->SetInputDataObject(pd);
    vrActor = vtkActor::New();
    vrActor->SetMapper(vrMapper);
    vrActor->SetProperty(actor->GetProperty());
    /* The new vtkActor pointer must be returned here */
    return vrActor;
}

void ModelPart::setTopLevelBool(bool topLevelBool)
{
    topLevel = topLevelBool;
}

bool ModelPart::getTopLevelBool() const
{
    return topLevel;
}

QVector3D ModelPart::getOriginalPosition() const {
    return originalPosition;
}
void ModelPart::setPosition(const QVector3D& newPosition) {
    position = newPosition;
    actor->SetPosition(position.x(), position.y(), position.z());
}
void ModelPart::resetToOriginalPosition() {
    actor->SetPosition(originalPosition.x(), originalPosition.y(), originalPosition.z());
}

void ModelPart::resetPosition() {
    setPosition(originalPosition);
}