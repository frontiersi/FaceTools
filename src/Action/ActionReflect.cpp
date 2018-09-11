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

#include <ActionReflect.h>
#include <ObjModelReflector.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionReflect;
using FaceTools::Action::EventSet;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;


ActionReflect::ActionReflect( const QString& dn, const QIcon& ico, QProgressBar* pb)
    : FaceAction(dn, ico)
{
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionReflect::testReady( const FV* fv) { return fv->data()->centreSet();}


bool ActionReflect::doAction( FVS& fvs, const QPoint&)
{
    assert(fvs.size() == 1);
    FV* fv = fvs.first();
    FM* fm = fv->data();

    fm->lockForWrite();

    const RFeatures::Orientation& on = fm->orientation();
    RFeatures::ObjModelInfo::Ptr info = fm->info();
    RFeatures::ObjModel::Ptr model = info->model();

    RFeatures::ObjModelReflector reflector( model);
    reflector.reflect( fm->centre(), on.up().cross( on.norm()));
    info->reset( model);
    fm->update(info);

    fm->unlock();
    return true;
}   // end doAction
