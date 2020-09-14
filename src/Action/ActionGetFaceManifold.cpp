/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionGetFaceManifold.h>
#include <QMessageBox>
#include <FaceModel.h>
#include <cassert>
using FaceTools::Action::ActionGetFaceManifold;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Landmark::LandmarkSet;
using MS = FaceTools::Action::ModelSelector;


ActionGetFaceManifold::ActionGetFaceManifold( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setAsync(true);
    addRefreshEvent( Event::MESH_CHANGE);
}   // end ctor


// public static
bool ActionGetFaceManifold::removeNonFaceManifolds( FM* fm)
{
    assert(fm);
    r3d::Mesh::Ptr mobj;
    fm->lockForWrite();
    const int mid = fm->faceManifoldId();
    if ( mid >= 0)
    {
        r3d::Copier copier(fm->mesh());
        const IntSet& fids = fm->manifolds()[mid].faces();
        for ( int fid : fids)
            copier.add(fid);
        mobj = copier.copiedMesh();
    }   // end if
    fm->update( mobj, true, true);
    fm->unlock();
    return mid >= 0;
}   // end removeNonFaceManifolds


bool ActionGetFaceManifold::isAllowed( Event)
{
    const FM* fm = MS::selectedModel();
    if ( !fm)
        return false;
    fm->lockForRead();
    const LandmarkSet& lmks = fm->currentAssessment()->landmarks();
    const bool rval = !lmks.empty() && fm->manifolds().count() > 1;
    fm->unlock();
    return rval;
}   // end isAllowed


bool ActionGetFaceManifold::doBeforeAction( Event)
{
    const FM* fm = MS::selectedModel();
    fm->lockForRead();
    const int mid = fm->faceManifoldId();
    const size_t numManRemove = fm->manifolds().count() - 1;
    const size_t numFaceRemove = size_t(fm->mesh().numFaces()) - fm->manifolds()[mid].faces().size();
    fm->unlock();

    return QMessageBox::question( qobject_cast<QWidget*>(parent()),
                                  tr("Removing Non-Face Manifolds"),
                                  QString("%1 triangles from %2 manifolds will be removed. Continue?").arg(numFaceRemove).arg(numManRemove),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}   // end doBeforeAction


void ActionGetFaceManifold::doAction( Event)
{
    storeUndo(this, Event::MESH_CHANGE | Event::LANDMARKS_CHANGE);
    removeNonFaceManifolds( MS::selectedModel());
}   // end doAction


Event ActionGetFaceManifold::doAfterAction( Event)
{
    MS::showStatus("Finished removing non-face manifolds.", 5000);
    return Event::MESH_CHANGE | Event::LANDMARKS_CHANGE;
}   // end doAfterAction

