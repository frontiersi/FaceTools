/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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


ResizeDialog::ResizeDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::ResizeDialog)
{
    _ui->setupUi(this);
    setModal(true);
    setWindowTitle( parent->windowTitle() + " | Resize Model");
    //setWindowFlags( windowFlags() & ~Qt::WindowCloseButtonHint);

    connect( _ui->resizeXSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::_doOnScaleXSpinBoxChanged);
    connect( _ui->resizeYSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::_doOnScaleYSpinBoxChanged);
    connect( _ui->resizeZSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::_doOnScaleZSpinBoxChanged);

    connect( _ui->newXSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::_doOnNewXSpinBoxChanged);
    connect( _ui->newYSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::_doOnNewYSpinBoxChanged);
    connect( _ui->newZSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::_doOnNewZSpinBoxChanged);

    setFixedSize( geometry().width(), geometry().height());
    reset(nullptr);
}   // end ctor


ResizeDialog::~ResizeDialog() { delete _ui;}


double ResizeDialog::xScaleFactor() const
{
    const double d = _ui->oldXLabel->text().toDouble();
    if ( d > 0.0)
        return _ui->newXSpinBox->value() / d;
    return 1.0;
}   // end xScaleFactor


double ResizeDialog::yScaleFactor() const
{
    const double d = _ui->oldYLabel->text().toDouble();
    if ( d > 0.0)
        return _ui->newYSpinBox->value() / d;
    return 1.0;
}   // end yScaleFactor


double ResizeDialog::zScaleFactor() const
{
    const double d = _ui->oldZLabel->text().toDouble();
    if ( d > 0.0)
        return _ui->newZSpinBox->value() / d;
    return 1.0;
}   // end zScaleFactor


void ResizeDialog::reset( const FM* fm)
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    if ( fm)
    {
        const r3d::Bounds& bnds = *fm->bounds()[0];
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
    _doOnScaleXSpinBoxChanged();  // Will update the Y and Z new size spin boxes too since X lock is checked.

    _checkCanApply();
}   // end reset


void ResizeDialog::_checkCanApply()
{
    const double x = xScaleFactor();
    const double y = yScaleFactor();
    const double z = zScaleFactor();
    _ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled( x != 1.0 || y != 1.0 || z != 1.0);
}   // end _checkCanApply


void ResizeDialog::_blockSpinBoxSignals( bool v)
{
    _ui->resizeXSpinBox->blockSignals( v);
    _ui->resizeYSpinBox->blockSignals( v);
    _ui->resizeZSpinBox->blockSignals( v);
    _ui->newXSpinBox->blockSignals( v);
    _ui->newYSpinBox->blockSignals( v);
    _ui->newZSpinBox->blockSignals( v);
}   // end _blockSpinBoxSignals


void ResizeDialog::_updateXLocked( double sf)
{
    if ( _ui->lockXButton->isChecked()) // Locked? Then update the other dimensions by the same scale factor to keep aspect ratio.
    {
        if ( _ui->lockYButton->isChecked())
        {
            _ui->resizeYSpinBox->setValue(sf);
            _ui->newYSpinBox->setValue( sf * _ui->oldYLabel->text().toDouble());
        }   // end if
        if ( _ui->lockZButton->isChecked())
        {
            _ui->resizeZSpinBox->setValue(sf);
            _ui->newZSpinBox->setValue( sf * _ui->oldZLabel->text().toDouble());
        }   // end if
    }   // end if
    _checkCanApply();
}   // end _updateXLocked


void ResizeDialog::_updateYLocked( double sf)
{
    if ( _ui->lockYButton->isChecked()) // Locked? Then update the other dimensions by the same scale factor to keep aspect ratio.
    {
        if ( _ui->lockXButton->isChecked())
        {
            _ui->resizeXSpinBox->setValue(sf);
            _ui->newXSpinBox->setValue( sf * _ui->oldXLabel->text().toDouble());
        }   // end if
        if ( _ui->lockZButton->isChecked())
        {
            _ui->resizeZSpinBox->setValue(sf);
            _ui->newZSpinBox->setValue( sf * _ui->oldZLabel->text().toDouble());
        }   // end if
    }   // end if
    _checkCanApply();
}   // end _updateYLocked


void ResizeDialog::_updateZLocked( double sf)
{
    if ( _ui->lockZButton->isChecked()) // Locked? Then update the other dimensions by the same scale factor to keep aspect ratio.
    {
        if ( _ui->lockXButton->isChecked())
        {
            _ui->resizeXSpinBox->setValue(sf);
            _ui->newXSpinBox->setValue( sf * _ui->oldXLabel->text().toDouble());
        }   // end if
        if ( _ui->lockYButton->isChecked())
        {
            _ui->resizeYSpinBox->setValue(sf);
            _ui->newYSpinBox->setValue( sf * _ui->oldYLabel->text().toDouble());
        }   // end if
    }   // end if
    _checkCanApply();
}   // end _updateZLocked


void ResizeDialog::_doOnScaleXSpinBoxChanged()
{
    const double sf = _ui->resizeXSpinBox->value();
    _blockSpinBoxSignals( true);
    _ui->newXSpinBox->setValue( sf * _ui->oldXLabel->text().toDouble());
    _updateXLocked(sf);
    _blockSpinBoxSignals( false);
}   // end _doOnXSpinBoxChanged


void ResizeDialog::_doOnScaleYSpinBoxChanged()
{
    const double sf = _ui->resizeYSpinBox->value();
    _blockSpinBoxSignals( true);
    _ui->newYSpinBox->setValue( sf * _ui->oldYLabel->text().toDouble());
    _updateYLocked(sf);
    _blockSpinBoxSignals( false);
}   // end _doOnYSpinBoxChanged


void ResizeDialog::_doOnScaleZSpinBoxChanged()
{
    const double sf = _ui->resizeZSpinBox->value();
    _blockSpinBoxSignals( true);
    _ui->newZSpinBox->setValue( sf * _ui->oldZLabel->text().toDouble());
    _updateZLocked(sf);
    _blockSpinBoxSignals( false);
}   // end _doOnZSpinBoxChanged


void ResizeDialog::_doOnNewXSpinBoxChanged()
{
    const double sf = xScaleFactor();
    _blockSpinBoxSignals( true);
    _ui->resizeXSpinBox->setValue( QString::number( sf, 'f', 2).toDouble());
    _updateXLocked( sf);
    _blockSpinBoxSignals( false);
}   // end _doOnXLineEdited


void ResizeDialog::_doOnNewYSpinBoxChanged()
{
    const double sf = yScaleFactor();
    _blockSpinBoxSignals( true);
    _ui->resizeYSpinBox->setValue( QString::number( sf, 'f', 2).toDouble());
    _updateYLocked( sf);
    _blockSpinBoxSignals( false);
}   // end _doOnYLineEdited


void ResizeDialog::_doOnNewZSpinBoxChanged()
{
    const double sf = zScaleFactor();
    _blockSpinBoxSignals( true);
    _ui->resizeZSpinBox->setValue( QString::number( sf, 'f', 2).toDouble());
    _updateZLocked( sf);
    _blockSpinBoxSignals( false);
}   // end _doOnZLineEdited
