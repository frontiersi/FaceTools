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

#include <ActionShowSelected.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
using FaceTools::Action::ActionShowSelected;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Vis::OutlinesView;

// public
ActionShowSelected::ActionShowSelected() : FaceAction(false)   // Don't lock before others
{
    // The change events that necessitate recalculating model boundaries.
    addRespondTo( MODEL_GEOMETRY_CHANGED);
    addRespondTo( MODEL_TRANSFORMED);
}   // end ctor


void ActionShowSelected::tellSelected( FaceControl* fc, bool enable)
{
    FaceModel* fm = fc->data();
    if ( _outlines.count(fm) == 0)  // Generate an outline if one not available
        _outlines[fm] = new OutlinesView( fm->model());
    _outlines.at(fm)->setVisible( enable, fc->viewer());
}   // end tellSelected


void ActionShowSelected::respondToChange( FaceControl* fc)
{
    FaceModel* fm = fc->data();
    bool vis = false;
    if ( _outlines.count(fm) > 0)
    {
        vis = _outlines.at(fm)->isVisible();
        delete _outlines.at(fm);    // Destructor removes from associated viewers
    }   // end if
    _outlines[fm] = new OutlinesView( fm->model());
    _outlines.at(fm)->setVisible( vis, fc->viewer());
}   // end respondToChange
