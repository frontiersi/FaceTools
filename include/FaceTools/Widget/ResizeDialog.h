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

#ifndef FACETOOLS_WIDGET_RESIZE_DIALOG_H
#define FACETOOLS_WIDGET_RESIZE_DIALOG_H

#include <FaceTools/FaceTypes.h>
#include <QDialog>

namespace Ui { class ResizeDialog;}

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT ResizeDialog : public QDialog
{ Q_OBJECT
public:
    explicit ResizeDialog( QWidget *parent=nullptr);
    ~ResizeDialog() override;

    void reset( const FM*);

    double xScaleFactor() const;
    double yScaleFactor() const;
    double zScaleFactor() const;

private:
    void _doOnScaleXSpinBoxChanged();
    void _doOnScaleYSpinBoxChanged();
    void _doOnScaleZSpinBoxChanged();
    void _doOnNewXSpinBoxChanged();
    void _doOnNewYSpinBoxChanged();
    void _doOnNewZSpinBoxChanged();

private:
    Ui::ResizeDialog *_ui;

    void _updateXLocked( double);
    void _updateYLocked( double);
    void _updateZLocked( double);
    void _blockSpinBoxSignals(bool);
    void _checkCanApply();
};  // end class

}}   // end namespace

#endif
