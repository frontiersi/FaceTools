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

#include <ActionSelect.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
using FaceTools::Interactor::ModelSelectInteractor;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::ActionSelect;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Vis::CuboidView;

// public
ActionSelect::ActionSelect() : _selector(true)  /* exclusive select */
{
    // The change events that necessitate recalculating the selection
    addRespondTo( DATA_CHANGE);
    setExternalSelect(false);   // Disallow since this action generates the selection events!
    connect( &_selector, &ModelSelectInteractor::onSelected, this, &ActionSelect::doOnSelected);
}   // end ctor


// public
void ActionSelect::setViewer( FaceModelViewer* v) { _selector.setViewer(v);}


// public
FaceModelViewer* ActionSelect::viewer() const
{
    return static_cast<FaceModelViewer*>(_selector.viewer());
}   // end viewer



// public
void ActionSelect::select( FaceControl* fc, bool enable)
{
    if ( _selector.isSelected(fc) == enable)    // Do nothing if no change in selection
        return;
    displaySelected( fc, enable);
    _selector.setSelected( fc, enable);
}   // end select


// public
void ActionSelect::setSelectEnabled( bool enable)
{
    _selector.enableUserSelect(enable);
}   // end setSelectEnabled


void ActionSelect::respondTo( const FaceAction*, const ChangeEventSet*, FaceControl* fc)
{
    FaceModel* fm = fc->data();
    bool vis = false;
    if ( _outlines.count(fm) > 0)
    {
        vis = _outlines.at(fm)->isVisible();
        delete _outlines.at(fm);    // Destructor removes from associated viewers
    }   // end if
    _outlines[fm] = new CuboidView( fm);
    _outlines.at(fm)->setVisible( vis, fc->viewer());
    fc->viewer()->updateRender();
}   // end respondTo


// private
void ActionSelect::displaySelected( const FaceControl* fc, bool enable)
{
    FaceModel* fm = fc->data();
    if ( _outlines.count(fm) == 0)  // Generate an outline if one not available
        _outlines[fm] = new CuboidView( fm);
    _outlines.at(fm)->setVisible( enable, fc->viewer());
    fc->viewer()->updateRender();
}   // end displaySelected


// private slot
void ActionSelect::doOnSelected( FaceControl* fc, bool enable)
{
    displaySelected( fc, enable);
    emit onSelect( fc, enable);
}   // end doOnSelected


