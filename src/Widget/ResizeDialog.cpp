/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
 *
 * Cliniface is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cliniface is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <Widget/ResizeDialog.h>
#include <ui_ResizeDialog.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QPushButton>
using FaceTools::Widget::ResizeDialog;
using FaceTools::FM;


// public
ResizeDialog::ResizeDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::ResizeDialog)
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Resize Model");
    setWindowFlags( windowFlags() & ~Qt::WindowCloseButtonHint);

    connectSpinBoxes();

    reset(nullptr);
}   // end ctor


// public
ResizeDialog::~ResizeDialog()
{
    delete _ui;
}   // end dtor


// public
double ResizeDialog::xScaleFactor() const
{
    const double d = _ui->oldXLabel->text().toDouble();
    if ( d > 0.0)
        return _ui->newXSpinBox->value() / d;
    return 1.0;
}   // end xScaleFactor


// public
double ResizeDialog::yScaleFactor() const
{
    const double d = _ui->oldYLabel->text().toDouble();
    if ( d > 0.0)
        return _ui->newYSpinBox->value() / d;
    return 1.0;
}   // end yScaleFactor


// public
double ResizeDialog::zScaleFactor() const
{
    const double d = _ui->oldZLabel->text().toDouble();
    if ( d > 0.0)
        return _ui->newZSpinBox->value() / d;
    return 1.0;
}   // end zScaleFactor


// public
void ResizeDialog::reset( const FM* fm)
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    if ( fm)
    {
        const RFeatures::ObjModelBounds& bnds = *fm->bounds()[0];
        x = bnds.xlen();
        y = bnds.ylen();
        z = bnds.zlen();
    }   // end if

    _ui->oldXLabel->setText( QString::number(x, 'f', 2));
    _ui->oldYLabel->setText( QString::number(y, 'f', 2));
    _ui->oldZLabel->setText( QString::number(z, 'f', 2));

    _ui->resizeXSpinBox->setValue(1.0);
    _ui->resizeYSpinBox->setValue(1.0);
    _ui->resizeZSpinBox->setValue(1.0);

    _ui->lockXButton->setChecked(true);
    _ui->lockYButton->setChecked(true);
    _ui->lockZButton->setChecked(true);
    doOnScaleXSpinBoxChanged();  // Will update the Y and Z new size spin boxes too since X lock is checked.

    checkCanApply();
}   // end reset


void ResizeDialog::checkCanApply()
{
    const double x = xScaleFactor();
    const double y = yScaleFactor();
    const double z = zScaleFactor();
    _ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled( x != 1.0 || y != 1.0 || z != 1.0);
}   // end checkCanApply


void ResizeDialog::connectSpinBoxes()
{
    connect( _ui->resizeXSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnScaleXSpinBoxChanged);
    connect( _ui->resizeYSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnScaleYSpinBoxChanged);
    connect( _ui->resizeZSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnScaleZSpinBoxChanged);

    connect( _ui->newXSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnNewXSpinBoxChanged);
    connect( _ui->newYSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnNewYSpinBoxChanged);
    connect( _ui->newZSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnNewZSpinBoxChanged);
}   // end connectSpinBoxes


void ResizeDialog::disconnectSpinBoxes()
{
    disconnect( _ui->resizeXSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnScaleXSpinBoxChanged);
    disconnect( _ui->resizeYSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnScaleYSpinBoxChanged);
    disconnect( _ui->resizeZSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnScaleZSpinBoxChanged);

    disconnect( _ui->newXSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnNewXSpinBoxChanged);
    disconnect( _ui->newYSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnNewYSpinBoxChanged);
    disconnect( _ui->newZSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::doOnNewZSpinBoxChanged);
}   // end disconnectSpinBoxes


void ResizeDialog::updateXLocked( double sf)
{
    if ( _ui->lockXButton->isChecked()) // Locked? Then update the other dimensions by the same scale factor to keep aspect ratio.
    {
        _ui->resizeYSpinBox->setValue(sf);
        _ui->resizeZSpinBox->setValue(sf);
        _ui->newYSpinBox->setValue( sf * _ui->oldYLabel->text().toDouble());
        _ui->newZSpinBox->setValue( sf * _ui->oldZLabel->text().toDouble());
    }   // end if
    checkCanApply();
}   // end updateXLocked


void ResizeDialog::updateYLocked( double sf)
{
    if ( _ui->lockYButton->isChecked()) // Locked? Then update the other dimensions by the same scale factor to keep aspect ratio.
    {
        _ui->resizeXSpinBox->setValue(sf);
        _ui->resizeZSpinBox->setValue(sf);
        _ui->newXSpinBox->setValue( sf * _ui->oldXLabel->text().toDouble());
        _ui->newZSpinBox->setValue( sf * _ui->oldZLabel->text().toDouble());
    }   // end if
    checkCanApply();
}   // end updateYLocked


void ResizeDialog::updateZLocked( double sf)
{
    if ( _ui->lockZButton->isChecked()) // Locked? Then update the other dimensions by the same scale factor to keep aspect ratio.
    {
        _ui->resizeXSpinBox->setValue(sf);
        _ui->resizeYSpinBox->setValue(sf);
        _ui->newXSpinBox->setValue( sf * _ui->oldXLabel->text().toDouble());
        _ui->newYSpinBox->setValue( sf * _ui->oldYLabel->text().toDouble());
    }   // end if
    checkCanApply();
}   // end updateZLocked


void ResizeDialog::doOnScaleXSpinBoxChanged()
{
    const double sf = _ui->resizeXSpinBox->value();
    _ui->newXSpinBox->setValue( sf * _ui->oldXLabel->text().toDouble());
    updateXLocked(sf);
}   // end doOnXSpinBoxChanged


void ResizeDialog::doOnScaleYSpinBoxChanged()
{
    const double sf = _ui->resizeYSpinBox->value();
    _ui->newYSpinBox->setValue( sf * _ui->oldYLabel->text().toDouble());
    updateYLocked(sf);
}   // end doOnYSpinBoxChanged


void ResizeDialog::doOnScaleZSpinBoxChanged()
{
    const double sf = _ui->resizeZSpinBox->value();
    _ui->newZSpinBox->setValue( sf * _ui->oldZLabel->text().toDouble());
    updateZLocked(sf);
}   // end doOnZSpinBoxChanged


void ResizeDialog::doOnNewXSpinBoxChanged()
{
    const double sf = xScaleFactor();
    disconnectSpinBoxes();
    _ui->resizeXSpinBox->setValue( QString::number( sf, 'f', 2).toDouble());
    updateXLocked( sf);
    connectSpinBoxes();
}   // end doOnXLineEdited


void ResizeDialog::doOnNewYSpinBoxChanged()
{
    const double sf = yScaleFactor();
    disconnectSpinBoxes();
    _ui->resizeYSpinBox->setValue( QString::number( sf, 'f', 2).toDouble());
    updateYLocked( sf);
    connectSpinBoxes();
}   // end doOnYLineEdited


void ResizeDialog::doOnNewZSpinBoxChanged()
{
    const double sf = zScaleFactor();
    disconnectSpinBoxes();
    _ui->resizeZSpinBox->setValue( QString::number( sf, 'f', 2).toDouble());
    updateZLocked( sf);
    connectSpinBoxes();
}   // end doOnZLineEdited
