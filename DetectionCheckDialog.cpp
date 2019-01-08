#include "DetectionCheckDialog.h"
#include "ui_DetectionCheckDialog.h"

DetectionCheckDialog::DetectionCheckDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DetectionCheckDialog)
{
    ui->setupUi(this);
}

DetectionCheckDialog::~DetectionCheckDialog()
{
    delete ui;
}
