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
#include <FaceModel.h>
using FaceTools::Report::SampleReport;
using RModelIO::LaTeXU3DInserter;
using RFeatures::CameraParams;
using FaceTools::FM;


LaTeXU3DInserter::Ptr SampleReport::createFigure( size_t, const std::string& tdir, const FM* fm, const CameraParams& cam)
{
    // Currently, the model itself is written out, not the VTK view (next version!)
    float widthMM = 160;
    float heightMM = 160;
    const RFeatures::ObjModel* model = fm->info()->cmodel();
    LaTeXU3DInserter::Ptr fig = LaTeXU3DInserter::create( model, tdir, widthMM, heightMM, cam, "Front Profile", "", true, false);
    if ( !fig)
        std::cerr << "Failed to create U3D model from data!" << std::endl;
    return fig;
}   // end createFigure
