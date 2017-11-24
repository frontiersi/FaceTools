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

#include <ActionMoveFace.h>
#include <FaceTools.h>
using FaceTools::ActionMoveFace;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionMoveFace::ActionMoveFace( const std::string& dname, const std::string& fname)
    : FaceTools::ActionProcessModel( dname, fname, false, true, true)
{
}   // end ctor


bool ActionMoveFace::operator()( FaceControl* fcont)
{
    FaceModel* fmodel = fcont->getModel();
    FaceTools::transformToOrigin( fmodel->getObjectMeta());
    fcont->updateMesh( fmodel->getObjectMeta()->getObject());
    return true;
}   // end operator()


bool ActionMoveFace::isActionable( FaceControl* fcont) const
{
    return FaceTools::hasReqLandmarks( fcont->getModel()->getObjectMeta());
}   // end isActionable

