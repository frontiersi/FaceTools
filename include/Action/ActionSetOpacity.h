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

#ifndef FACE_TOOLS_ACTION_SET_OPACITY_H
#define FACE_TOOLS_ACTION_SET_OPACITY_H

#include "FaceAction.h"
#include <QDoubleSpinBox>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionSetOpacity : public FaceAction
{ Q_OBJECT
public:
    // Display name will be used for spin box widget's tool tip.
    explicit ActionSetOpacity( const QString&);

    QString toolTip() const override { return "Change the opacity of model surfaces.";}

    QWidget* widget() const override { return _opacitySpinBox;}

    // Sets a new value for opacity on overlap. Fixes the value between minOpacity and 1.0 and returns it.
    static void setOpacityOnOverlap( double);
    static void setMinOpacity( double);

protected:
    void postInit() override;
    bool checkEnable( Event) override;
    void doAction( Event) override;

private slots:
    void doOnValueChanged( double);

private:
    static double s_maxOpacityOnOverlap;
    static double s_minOpacity;
    QDoubleSpinBox *_opacitySpinBox;
};  // end class

}}   // end namespaces

#endif
