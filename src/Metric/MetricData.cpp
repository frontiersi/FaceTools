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

#include <MetricData.h>
#include <LandmarksManager.h>
#include <FaceModel.h>
#include <sstream>
using FaceTools::Metric::MetricData;
using FaceTools::Metric::MCTI;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::FM;
using FaceTools::FaceLateral;


MetricData::MetricData()


void MetricData::setSource( const std::string& s) { _source = s;}
void MetricData::setNote( const std::string& n) { _note = n;}
void MetricData::setEthnicities( const std::string& d) { _ethnicities = d;}
void MetricData::setSex( FaceTools::Sex s) { _sex = s;}
void MetricData::setRSD( size_t, rlib::RSD::Ptr rsd) { _rsd = rsd;}
