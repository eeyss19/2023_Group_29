#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QColorDialog>
#include <QCheckBox>
#include <QSettings>
#include "ModelPart.h"

namespace Ui {
class OptionDialog;
}
class OptionDialog : public QDialog
{
    Q_OBJECT

public:

    explicit OptionDialog(QWidget *parent = nullptr);
    ~OptionDialog();
    void set_ptr(ModelPart* Pointer);

public slots:

    void updateModelPartName(const QString &name);
    void updateModelPartColor();
    void updateModelPartVisibility(int state);
    void saveSettings();
    void loadSettings();
signals:

    void settingsSaved();

private:
    Ui::OptionDialog *ui;
    ModelPart *ptr = nullptr;
    QString Name = "Enter:";
    QColor Colour;
    bool isVisible = true;
};

#endif // OPTIONDIALOG_H
