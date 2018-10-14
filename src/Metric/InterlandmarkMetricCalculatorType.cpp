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

#include <InterlandmarkMetricCalculatorType.h>
#include <LandmarksManager.h>
#include <FaceModel.h>
#include <sstream>
using FaceTools::Metric::InterlandmarkMetricCalculatorType;
using FaceTools::Metric::MCTI;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::FM;
using FaceTools::FaceLateral;


InterlandmarkMetricCalculatorType::InterlandmarkMetricCalculatorType()
    : _id(-1), _bilat(false), _lA(-1), _lB(-1), _edv( -1, -1, -1), _ndps(0) {}


// private
InterlandmarkMetricCalculatorType::InterlandmarkMetricCalculatorType( int lA, int lB, bool b)
    : _id(-1), _bilat(b), _lA(lA), _lB(lB), _edv( -1, lA, lB, b), _ndps(0)
{
}   // end ctor


std::string InterlandmarkMetricCalculatorType::params() const
{
    return (QString("%1").arg(_lA) + " " + QString("%1").arg(_lB)).toStdString();
}   // end params


MCTI::Ptr InterlandmarkMetricCalculatorType::fromParams( const std::string& prms) const
{
    std::istringstream iss(prms);
    std::string cA, cB;
    iss >> cA;
    cB = cA;
    if ( iss.good())
        iss >> cB;

    if ( !LDMKS_MAN::hasCode(cA.c_str()) || !LDMKS_MAN::hasCode(cB.c_str()))
    {
        std::cerr << "[WARNING] FaceTools::Metric::InterlandmarkMetricCalculatorType::fromParams: "
                  << "Landmarks " << cA << " and/or " << cB << " not found!" << std::endl;
        return nullptr;
    }   // end if

    FaceTools::Landmark::Landmark* lmkA = LDMKS_MAN::landmark(cA.c_str());
    FaceTools::Landmark::Landmark* lmkB = LDMKS_MAN::landmark(cB.c_str());

    // Non-bilateral landmarks CANNOT be the same since this denotes a point.
    if ( lmkA == lmkB && !lmkA->isBilateral() && !lmkB->isBilateral())
    {
        std::cerr << "[WARNING] FaceTools::Metric::InterlandmarkMetricCalculatorType::fromParams: "
                  << "Cannot create interlandmark distance calculator for landmark if it's non-bilateral!" << std::endl;
        return nullptr;
    }   // end if

    // If lA and lB are the same, the measurement cannot be bilateral even if the landmarks are.
    const bool bilat = (lmkA != lmkB) && (lmkA->isBilateral() || lmkB->isBilateral());
    return MCTI::Ptr( new InterlandmarkMetricCalculatorType( lmkA->id(), lmkB->id(), bilat));
}   // end fromParams


void InterlandmarkMetricCalculatorType::setId( int id) { _id = id; _edv.setMetricId(_id);}
void InterlandmarkMetricCalculatorType::setName( const std::string& nm) { _name = nm;}
void InterlandmarkMetricCalculatorType::setDescription( const std::string& ds) { _desc = ds;}
void InterlandmarkMetricCalculatorType::setSource( const std::string& s) { _source = s;}
void InterlandmarkMetricCalculatorType::setEthnicities( const std::string& d) { _ethnicities = d;}
void InterlandmarkMetricCalculatorType::setSex( FaceTools::Sex s) { _sex = s;}
void InterlandmarkMetricCalculatorType::setNumDecimals( size_t ndps) { _ndps = ndps;}
void InterlandmarkMetricCalculatorType::setRSD( size_t, rlib::RSD::Ptr rsd) { _rsd = rsd;}


bool InterlandmarkMetricCalculatorType::isAvailable( const FM* fm) const
{
    LandmarkSet::Ptr lmks = fm->landmarks();
    return lmks->has(_lA) && lmks->has(_lB);
}   // end isAvailable


double InterlandmarkMetricCalculatorType::measure( size_t, const FM* fm, FaceLateral lat) const
{
    assert( isAvailable(fm));
    LandmarkSet::Ptr lmks = fm->landmarks();

    FaceLateral latA = lat;
    FaceLateral latB = lat;

    if ( isBilateral())
    {
        if ( lat == FACE_LATERAL_MEDIAL)
        {
            std::cerr << "[ERROR] FaceTools::Metric::InterlandmarkMetricCalculatorType::measure: Must specify left or right lateral for bilateral measure!" << std::endl;
            assert(false);
        }   // end if

        latA = LDMKS_MAN::landmark(_lA)->isBilateral() ? lat : FACE_LATERAL_MEDIAL;
        latB = LDMKS_MAN::landmark(_lB)->isBilateral() ? lat : FACE_LATERAL_MEDIAL;
        assert( latA != FACE_LATERAL_MEDIAL || latB != FACE_LATERAL_MEDIAL);    // Must be true of wouldn't be a bilateral measurement
    }   // end if
    else
    {
        // If not a bilateral measure, then either both landmarks must be medial,
        // or the landmarks must be the same.
        if ( lat != FACE_LATERAL_MEDIAL)
        {
            std::cerr << "[ERROR] FaceTools::Metric::InterlandmarkMetricCalculatorType::measure: Cannot specify face lateral for non-bilateral measure!" << std::endl;
            assert(false);
        }   // end if

        if ( _lA == _lB)
        {
            latA = FACE_LATERAL_LEFT;
            latB = FACE_LATERAL_RIGHT;
        }   // end if
    }   // end else

    return cv::norm( *lmks->pos(_lA, latA) - *lmks->pos(_lB, latB));
}   // end measure
