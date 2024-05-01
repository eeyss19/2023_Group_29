/**     @file ModelPart.h
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Template for model parts that will be added as treeview items
  *
  *     P Evans 2022
  */
  
#ifndef VIEWER_MODELPART_H
#define VIEWER_MODELPART_H

#include <QString>
#include <QList>
#include <QVariant>
#include <QColorDialog>
#include <vtkSmartPointer.h>
#include <vtkMapper.h>
#include <vtkActor.h>
#include <vtkSTLReader.h>
#include <vtkColor.h>

/* VTK headers - will be needed when VTK used in next worksheet,
 * commented out for now
 *
 * Note that there are a few function definitions and variables
 * commented out below - this is because you haven't yet installed
 * the VTK library which is needed.
 */

class ModelPart {
public:
    /** Constructor
     * @param data is a List (array) of strings for each property of this item (part name and visiblity in our case
     * @param parent is the parent of this item (one level up in tree)
     */
    ModelPart(const QList<QVariant>& data, ModelPart* parent = nullptr);

    /** Destructor
      * Needs to free array of child items
      */
    ~ModelPart();

    /** Add a child to this item.
      * @param item Pointer to child object (must already be allocated using new)
      */
    void appendChild(ModelPart* item);

    /** Return child at position 'row' below this item
      * @param row is the row number (below this item)
      * @return pointer to the item requested.
      */
    ModelPart* child(int row);

    /** Return number of children to this item
      * @return number of children
      */
    int childCount() const;         /* Note on the 'const' keyword - it means that this function is valid for
                                     * constant instances of this class. If a class is declared 'const' then it
                                     * cannot be modifed, this means that 'set' type functions are usually not
                                     * valid, but 'get' type functions are.
                                     */

    /** Get number of data items (2 - part name and visibility string) in this case.
      * @return number of visible data columns
      */
    int columnCount() const;

    /** Return the data item at a particular column for this item.
      * i.e. either part name of visibility
      * used by Qt when displaying tree
      * @param column is column index
      * @return the QVariant (represents string)
      */
    QVariant data(int column) const;


    /** Default function required by Qt to allow setting of part
      * properties within treeview.
      * @param column is the index of the property to set
      * @param value is the value to apply
      */
    void set( int column, const QVariant& value );

    /** Get pointer to parent item
      * @return pointer to parent item
      */
    ModelPart* parentItem();

    /** Get row index of item, relative to parent item
      * @return row index
      */
    int row() const;


    /** Set colour
      * (0-255 RGB values as ints)
      */
    void setColour(QColor Clr);


    /** Set visible flag
      * @param isVisible sets visible/non-visible
      */
    void setVisible(bool state);
	
	/** Load STL file
      * @param fileName
      */
    void loadSTL(QString fileName);

    /**
     * Set the name of the ModelPart.
    * @param name is the new name for the ModelPart.
    */
    void setName(QString name);

    /**
     * Get the color of the ModelPart.
     * @return the color of the ModelPart as a QColor object.
     */
    QColor get_Color(void) const;

    /**
     * Get the name of the ModelPart.
     * @return the name of the ModelPart as a QString.
     */
    const QString get_Name(void);

    /**
     * Get the visibility status of the ModelPart.
     * @return the visibility status of the ModelPart as a boolean. True if the ModelPart is visible, false otherwise.
     */
    bool get_Visibility(void) const;

    /** Return actor
      * @return pointer to default actor for GUI rendering
      */
    vtkSmartPointer<vtkActor> getActor();

    vtkActor* getNewActor();

    
    /** Return new actor for use in VR
      * @return pointer to new actor
      */
    //vtkActor* getNewActor();
    
private:
    QList<ModelPart*>                           m_childItems;       /**< List (array) of child items */
    QList<QVariant>                             m_itemData;         /**< List (array of column data for item */
    ModelPart*                                  m_parentItem;       /**< Pointer to parent */

    /* These are some typical properties that I think the part will need, you might
     * want to add you own.
     */
    bool                                        isVisible;          /**< True/false to indicate if should be visible in model rendering */
    QColor                                      Colour = Qt::GlobalColor::red;
    QString                                     Name;
	/* These are vtk properties that will be used to load/render a model of this part,
	 * commented out for now but will be used later
	 */
	vtkSmartPointer<vtkSTLReader>               file;               /**< Datafile from which part loaded */
    vtkSmartPointer<vtkMapper>                  mapper;             /**< Mapper for rendering */
    vtkSmartPointer<vtkActor>                   actor;              /**< Actor for rendering */
    vtkSmartPointer<vtkMapper> newMapper;                   /**< Mapper for rendering in VR */
    vtkSmartPointer<vtkActor> newActor;                              /**< Actor for rendering in VR */
};  


#endif

