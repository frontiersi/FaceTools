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

#include <SampleReport.h>
using FaceTools::Report::SampleReport;
using RModelIO::LaTeXU3DInserter;
using RFeatures::CameraParams;
using RFeatures::ObjModel;
using FaceTools::FM;


LaTeXU3DInserter::Ptr SampleReport::createFigure( size_t, const QString& tdir, const ObjModel* cmodel, const CameraParams& cam)
{
    // Currently, the model itself is written out, not the VTK view (next version!)
    float widthMM = 180;
    float heightMM = 200;
    LaTeXU3DInserter::Ptr fig = LaTeXU3DInserter::create( cmodel, tdir.toStdString(), widthMM, heightMM, cam, "3D Model", "", true, false);
    if ( !fig)
        std::cerr << "Failed to create U3D model from data!" << std::endl;
    return fig;
}   // end createFigure
