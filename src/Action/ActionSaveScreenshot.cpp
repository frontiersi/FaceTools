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

#include <Action/ActionSaveScreenshot.h>
#include <FaceModelViewer.h>
#include <FeatureUtils.h>
#include <QImageTools.h>
#include <algorithm>
using FaceTools::Action::ActionSaveScreenshot;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FMV;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using MS = FaceTools::Action::ModelSelector;


ActionSaveScreenshot::ActionSaveScreenshot( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
}   // end ctor


void ActionSaveScreenshot::doAction( Event)
{
    std::vector<cv::Mat> imgs;
    for ( const FMV* fmv : MS::viewers())
    {
        QSize sz = fmv->size();
        if ( fmv->isVisible() && sz.width() > 0 && sz.height() > 0)
            imgs.push_back( fmv->grabImage());
    }   // end for

    cv::Mat img = RFeatures::concatHorizontalMax( imgs);
    QTools::saveImage( img);
}   // end doAction
