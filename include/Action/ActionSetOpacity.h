/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionSetOpacity : public FaceAction
{ Q_OBJECT
public:
    // Display name will be used for spin box widget's tool tip.
    ActionSetOpacity( const QString& dname, double maxOpacityOnOverlap=0.5, double minOpacity=0.1, QWidget* parent=nullptr);

    QWidget* getWidget() const override { return _opacitySpinBox;}

public slots:
    // Sets a new value for opacity on overlap. Fixes the value between minOpacity and 1.0 and returns it.
    double setOpacityOnOverlap( double);

private slots:
    void tellReady( const Vis::FV*, bool) override;
    bool testEnabled( const QPoint* mc=nullptr) const override;
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet& cs, const FVS&, bool) override { cs.insert(VIEW_CHANGE);}
    void doOnValueChanged( double);

private:
    double _maxOpacityOnOverlap;
    QDoubleSpinBox *_opacitySpinBox;
};  // end class

}   // end namespace
}   // end namespace

#endif
