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

#include <ActionInvertNormals.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionInvertNormals;
using FaceTools::Action::EventSet;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::FM;


ActionInvertNormals::ActionInvertNormals( const QString& dn, const QIcon& ico, QProgressBar* pb)
    : FaceAction(dn, ico)
{
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionInvertNormals::doAction( FVS& fvs, const QPoint&)
{
    FMS fms = fvs.models(); // Copy out
    fvs.clear();

    for ( FM* fm : fms)
    {
        fm->lockForWrite();

        RFeatures::ObjModelInfo::Ptr info = fm->info();
        RFeatures::ObjModel::Ptr model = info->model();

        const IntSet& fids = model->getFaceIds();
        std::for_each( std::begin(fids), std::end(fids), [&](int fid){ model->reverseFaceVertices(fid);});

        info->reset( model);
        fm->update(info);

        fm->unlock();

        fvs.insert(fm);
    }   // end for
    return true;
}   // end doAction
