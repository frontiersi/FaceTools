#include "ReportChooserDialog.h"
#include "ui_ReportChooserDialog.h"

ReportChooserDialog::ReportChooserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportChooserDialog)
{
    ui->setupUi(this);
}

ReportChooserDialog::~ReportChooserDialog()
{
    delete ui;
}
