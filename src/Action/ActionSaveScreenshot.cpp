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

#include <ActionSaveScreenshot.h>
#include <FaceModelViewer.h>
#include <FeatureUtils.h>
#include <QImageTools.h>
#include <algorithm>
using FaceTools::Action::ActionSaveScreenshot;
using FaceTools::Action::FaceAction;
using FaceTools::FMV;
using FaceTools::Vis::FV;
using FaceTools::FVS;


ActionSaveScreenshot::ActionSaveScreenshot( const QString& dn, const QIcon& ico, FMV *mv)
    : FaceAction( dn, ico)
{
    if ( mv)
        addViewer(mv);
}   // end ctor


bool ActionSaveScreenshot::doAction( FVS& fset, const QPoint&)
{
    if ( _viewers.empty())
    {
        for ( FV* fv : fset)
            fv->viewer()->saveScreenshot();
    }   // end if
    else
    {
        std::vector<cv::Mat> mimgs;
        for ( const FMV *v : _viewers)
        {
            if ( v->width() > 0 && v->height() > 0)
                mimgs.push_back( v->grabImage());
        }   // end for
        cv::Mat m = RFeatures::concatHorizontalMax( mimgs);
        QTools::saveImage( m);
    }   // end else

    return true;
}   // end doAction
