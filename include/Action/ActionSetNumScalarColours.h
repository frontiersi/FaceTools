/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef FACE_TOOLS_ACTION_SET_NUM_SCALAR_COLOURS_H
#define FACE_TOOLS_ACTION_SET_NUM_SCALAR_COLOURS_H

#include "FaceAction.h"
#include <QSpinBox>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionSetNumScalarColours : public FaceAction
{ Q_OBJECT
public:
    explicit ActionSetNumScalarColours( const QString&);

    QString toolTip() const override { return "Set the number of colours to interpolate over when mapping scalar visualisations to a model's surface.";}

    QWidget* widget() const override { return _spinBox;}

protected:
    void postInit() override;
    bool checkEnable( Event) override;

private:
    QSpinBox *_spinBox;
    void updateFaceViewFromWidget( int);
};  // end class

}}   // end namespaces

#endif
