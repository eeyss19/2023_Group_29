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

ModelPart::ModelPart(const QList<QVariant>& data, ModelPart* parent)
    : m_itemData(data), m_parentItem(parent) {
    file = vtkSmartPointer<vtkSTLReader>::New();
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
 * @brief Sets the color of the model part.
 *
 * @param Clr The color to set for the model part.
 */
void ModelPart::setColour(QColor Clr) {
    Colour = Clr;

}

/**
 * @brief Sets the visibility of the model part.
 *
 * @param isvisible The visibility status to set for the model part.
 */
void ModelPart::setVisible(bool isvisible) {
    isVisible = isvisible;
}

/**
 * @brief Loads an STL file for the model part.
 *
 * @param fileName The name of the STL file to load.
 */
void ModelPart::loadSTL(QString fileName) {
    // Load the STL file
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
}

/**
 * @brief Sets the name of the model part.
 *
 * @param name The name to set for the model part.
 */
void ModelPart::setName(QString name) {
    Name = name;
}

/**
 * @brief Gets the color of the model part.
 *
 * @return The color of the model part.
 */
QColor ModelPart::get_Color(void) const {
    return Colour;
}

/**
 * @brief Gets the name of the model part.
 *
 * @return The name of the model part.
 */
const QString ModelPart::get_Name(void) {
    return Name;
}

/**
 * @brief Gets the visibility status of the model part.
 *
 * @return The visibility status of the model part.
 */
bool ModelPart::get_Visibility(void) const {
    return isVisible;
}

/**
 * @brief Gets the actor of the model part.
 *
 * @return The actor of the model part.
 */
vtkSmartPointer<vtkActor> ModelPart::getActor() {
    return actor;
}



//vtkActor* ModelPart::getNewActor() {
/* This is a placeholder function that will be used in the next worksheet.
* 
* The default mapper/actor combination can only be used to render the part in 
* the GUI, it CANNOT also be used to render the part in VR. This means you need
* to create a second mapper/actor combination for use in VR - that is the role
* of this function. */
     
     
/* 1. Create new mapper */
     
/* 2. Create new actor and link to mapper */
     
/* 3. Link the vtkProperties of the original actor to the new actor. This means 
*    if you change properties of the original part (colour, position, etc), the
*    changes will be reflected in the GUI AND VR rendering.
*    
*    See the vtkActor documentation, particularly the GetProperty() and SetProperty()
*    functions.
*/
    

/* The new vtkActor pointer must be returned here */
//    return nullptr;
    
//}

