/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_RADIAL_SELECTOR_H
#define FACE_TOOLS_ACTION_RADIAL_SELECTOR_H

/**
 * A checkable action that enables an interface to select radial regions
 * on models. Once checked, the interface changes to the radial selection
 * tool which causes a selected radial area to be indicated centred around
 * the mouse cursor. 
 */

#include "FaceAction.h"
#include <BoundaryView.h>
#include <ObjModelRegionSelector.h>    // RFeatures

namespace FaceTools {
namespace Action {

class ActionRadialSelector : public FaceAction
{ Q_OBJECT
public:
    explicit ActionRadialSelector( const char* icon=":/icons/circle.png");

    QString getDisplayName() const override { return "Select Region";}
    const QIcon* getIcon() const override { return &_icon;}

    void setControlled( FaceControl*, bool) override;

signals:
    //void selectedRegion( FaceControl*

public slots:
    bool doAction() override;

private:
    const QIcon _icon;
    RFeatures::ObjModelRegionSelector::Ptr _cropper;
    RFeatures::ObjModel::Ptr _cobj; // The finished crop on the current object

    boost::unordered_set<FaceControl*> _fconts;
    BoundaryView *_bview;

    void setCropRadius( double);
};  // end class

}   // end namespace
}   // end namespace

#endif
