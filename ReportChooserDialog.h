#ifndef REPORTCHOOSERDIALOG_H
#define REPORTCHOOSERDIALOG_H

#include <QDialog>

namespace Ui {
class ReportChooserDialog;
}

class ReportChooserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportChooserDialog(QWidget *parent = nullptr);
    ~ReportChooserDialog();

private:
    Ui::ReportChooserDialog *ui;
};

#endif // REPORTCHOOSERDIALOG_H
