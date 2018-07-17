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

#include <ActionRemesh.h>
#include <FaceModelViewer.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <ObjModelVertexAdder.h>   // RFeatures
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionRemesh;
using FaceTools::Action::ChangeEventSet;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionRemesh::ActionRemesh( const QString& dn, const QIcon& ico, QProgressBar* pb)
    : FaceAction(dn, ico), _maxEdgeLen(2.0)
{
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionRemesh::doAction( FaceControlSet& rset)
{
    assert(rset.size() == 1);
    FaceControl* fc = rset.first();
    FaceModel* fm = fc->data();

    fm->lockForWrite();

    bool success = true;
    RFeatures::ObjModelInfo::Ptr info = fm->info();

    // Debug - work on a cloned copy until sure remeshing works as expected.
    RFeatures::ObjModel::Ptr cloned = RFeatures::ObjModel::copy( info->cmodel());

    RFeatures::ObjModelVertexAdder vadder( cloned);
    std::cerr << "[INFO] FaceTools::Action::ActionRemesh::doAction:: Adding vertices for max edge length of "
              << maxEdgeLength() << std::endl;
    vadder.subdivideEdges( maxEdgeLength());
    if ( info->reset( cloned))
        fm->update(info);
    else
    {
        std::cerr << "[ERROR] FaceTools::ActionRemesh::doAction: Unable to clean model post remesh!" << std::endl;
        success = false;
    }   // end else

    fm->unlock();
    return success;
}   // end doAction
