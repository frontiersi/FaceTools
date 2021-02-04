/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionRemoveManifolds.h>
#include <FaceModel.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::ActionRemoveManifolds;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;


ActionRemoveManifolds::ActionRemoveManifolds( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico), _mid(-1)
{
    addRefreshEvent( Event::MESH_CHANGE);
    setAsync(true);
}   // end ctor


namespace  {
bool manifoldIdInRange( int mid)
{
    bool inRng = false;
    if ( MS::isViewSelected())
    {
        const FM* fm = MS::selectedModel();
        inRng = mid >= 0 && mid < static_cast<int>(fm->manifolds().count());
    }   // end if
    return inRng;
}   // end manifoldIdInRange
}   // end namespace


bool ActionRemoveManifolds::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    // Ready if more than one manifold and the point is on the face.
    return fv && MS::interactionMode() == IMode::CAMERA_INTERACTION
              && (fv->data()->manifolds().count() > 1)
              && (manifoldIdInRange(_mid) || fv->isPointOnFace( primedMousePos()));
}   // end isAllowed


bool ActionRemoveManifolds::doBeforeAction( Event)
{
    const FV* fv = MS::selectedView();
    const FM* fm = fv->data();
    fm->lockForRead();

    Vec3f cpos;
    if ( fv->projectToSurface( primedMousePos(), cpos))
    {
        const int svidx = fm->findVertex( cpos);
        const int fx = *fm->mesh().faces(svidx).begin(); // Get an attached face 
        _mid = fm->manifolds().fromFaceId(fx); // Manifold ID
        assert( _mid >= 0);
    }   // end if

    const size_t numManRemove = fm->manifolds().count() - 1;
    const size_t numFaceRemove = manifoldIdInRange(_mid) ? size_t(fm->mesh().numFaces()) - fm->manifolds()[_mid].faces().size() : 0;
    fm->unlock();

    if ( numFaceRemove > 0)
    {
        const QString msg = tr("%1 triangles from %2 manifolds will be removed. Do you want to continue?").arg(numFaceRemove).arg(numManRemove);
        const int rv = QMB::question( qobject_cast<QWidget*>(parent()),
                              tr("Remove Other Manifolds?"), QString("<p align='center'>%1</p>").arg(msg),
                              QMB::Yes | QMB::No, QMB::No);
        if ( rv == QMB::No)
            _mid = -1;
    }   // end if

    if ( _mid >= 0)
    {
        MS::showStatus("Removing manifolds from selected model...");
        storeUndo(this, Event::MESH_CHANGE | Event::LANDMARKS_CHANGE);
    }   // end if

    return _mid >= 0;
}   // end doBeforeAction


void ActionRemoveManifolds::doAction( Event)
{
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    r3d::Copier copier( fm->mesh());
    const IntSet& fids = fm->manifolds()[_mid].faces();
    for ( int fid : fids)
        copier.add(fid);
    r3d::Mesh::Ptr mobj = copier.copiedMesh();
    fm->update( mobj, true, true);
    fm->unlock();
}   // end doAction


Event ActionRemoveManifolds::doAfterAction( Event)
{
    _mid = -1;
    MS::showStatus("Finished removing manifolds.", 5000);
    return Event::MESH_CHANGE | Event::LANDMARKS_CHANGE;
}   // end doAfterAction

