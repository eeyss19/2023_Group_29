#include "optiondialog.h"
#include "ui_optiondialog.h"
#include <QLineEdit>
#include <QColorDialog>
#include <QCheckBox>
#include <QColorDialog>
#include "ModelPart.h"
#include "mainwindow.h"

OptionDialog::OptionDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OptionDialog)
{
    ui->setupUi(this);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &OptionDialog::updateModelPartName);
    connect(ui->checkBox, &QCheckBox::stateChanged, this, &OptionDialog::updateModelPartVisibility);
    connect(ui->pushButton, &QPushButton::released, this, &OptionDialog::updateModelPartColor);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &OptionDialog::saveSettings);
    connect(this, &OptionDialog::settingsSaved, static_cast<MainWindow*>(parent), &MainWindow::updateRender);
    connect(this, &OptionDialog::settingsSaved, static_cast<MainWindow*>(parent), &MainWindow::update_name);

}

OptionDialog::~OptionDialog()
{
    delete ui;
}
void OptionDialog::updateModelPartName(const QString &name){
    Name = ui->lineEdit->text();
}
void OptionDialog::updateModelPartColor() {
    QColor newColour = QColorDialog::getColor(Colour, this, "Select colour", QColorDialog::DontUseNativeDialog);
    if (newColour != Colour) Colour = newColour;
}
void OptionDialog::updateModelPartVisibility(int state){
    if(state == Qt::Checked) isVisible = true;
    else isVisible = false;
}

void OptionDialog::saveSettings(){
    ptr -> setColour(Colour);
    ptr -> setVisible(ui->checkBox->isChecked());
    ptr -> setName(ui->lineEdit->text());
    emit settingsSaved();
}


void OptionDialog::loadSettings(){
    Colour = ptr->get_Color();
    Name = ptr->get_Name();
    ui->lineEdit->setText(Name);
    isVisible = ptr->get_Visibility();
    ui->checkBox->setChecked(isVisible);
}

void OptionDialog::set_ptr(ModelPart* Pointer){
    ptr = Pointer;
}
