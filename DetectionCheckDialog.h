#ifndef DETECTIONCHECKDIALOG_H
#define DETECTIONCHECKDIALOG_H

#include <QDialog>

namespace Ui {
class DetectionCheckDialog;
}

class DetectionCheckDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DetectionCheckDialog(QWidget *parent = nullptr);
    ~DetectionCheckDialog();

private:
    Ui::DetectionCheckDialog *ui;
};

#endif // DETECTIONCHECKDIALOG_H
