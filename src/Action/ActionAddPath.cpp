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

#include <ActionAddPath.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <FaceTools.h>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionAddPath;
using FaceTools::Action::ActionEditPaths;
using FaceTools::Action::EventSet;
using FaceTools::Interactor::PathSetInteractor;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;


ActionAddPath::ActionAddPath( const QString& dn, const QIcon& ico, ActionEditPaths* e)
    : FaceAction( dn, ico), _editor(e)
{
}   // end ctor


bool ActionAddPath::testEnabled( const QPoint*) const
{
    const bool g = _editor->interactor()->hoverPathId() < 0;
    const FV* fv = _editor->interactor()->hoverModel();
    return isReady(fv) && g;
}   // end testEnabled


bool ActionAddPath::doAction( FVS& fvs, const QPoint& p)
{
    assert(_editor);
    FV* fv = fvs.first();
    assert(fv);
    assert( _editor->interactor()->hoverModel() == fv);
    fvs.clear();

    FM* fm = fv->data();
    cv::Vec3f hpos;
    int pid = -1;
    if ( fv->projectToSurface(p, hpos))
    {
        //fm->lockForWrite();
        pid = fm->paths()->addPath(hpos);
        assert(pid >= 0);
        fm->setSaved(false);
        //fm->unlock();
        fvs.insert( fm);
        _editor->interactor()->setPathDrag( pid);

        // The point used to set the initial handle is not
        // necessarily where the mouse cursor is now.
        if ( fv->projectToSurface( fv->viewer()->mouseCoords(), hpos))
            _editor->interactor()->moveDragHandle( hpos);
    }   // end if

    return pid >= 0;
}   // end doAction
