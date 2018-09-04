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

#include <ActionTransformToStandardPosition.h>
#include <Transformer.h>  // RFeatures
#include <FaceModel.h>
#include <FaceTools.h>
#include <FaceView.h>
#include <VtkTools.h>
using FaceTools::Action::ActionTransformToStandardPosition;
using FaceTools::Action::FaceAction;
using FaceTools::FVS;
using FaceTools::FaceModel;


ActionTransformToStandardPosition::ActionTransformToStandardPosition( const QString &dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setRespondToEvent( ORIENTATION_CHANGE);
}   // end ctor


bool ActionTransformToStandardPosition::doAction( FVS& rset, const QPoint&)
{
    const FaceModelSet& fms = rset.models();
    for ( FaceModel* fm : fms)
    {
        fm->lockForWrite();
        const cv::Vec3f& c = fm->centre();
        const RFeatures::Orientation& on = fm->orientation();
        cv::Matx44d m = RFeatures::toStandardPosition( on.norm(), on.up(), c);

        std::cerr << "PRE-TRANSFORM:" << std::endl;
        std::cerr << "  Centre : " << c << std::endl;
        std::cerr << "  Orientation (norm,up)  : " << on << std::endl;

        fm->transform(m);

        std::cerr << "POST-TRANSFORM:" << std::endl;
        std::cerr << "  Centre : " << c << std::endl;
        std::cerr << "  Orientation (norm,up)  : " << on << std::endl;

        fm->unlock();
    }   // end for
    return true;
}   // end doAction
