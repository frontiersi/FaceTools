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

#include <DistanceMetricCalculatorType.h>
#include <LandmarksManager.h>
#include <FaceModel.h>
#include <sstream>
using FaceTools::Metric::DistanceMetricCalculatorType;
using FaceTools::Metric::MCTI;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::FM;
using FaceTools::FaceLateral;


const QString DistanceMetricCalculatorType::s_cat("Distance");


DistanceMetricCalculatorType::DistanceMetricCalculatorType()
    : _id(-1), _bilat(false), _lA(-1), _lB(-1), _edv( -1, -1, -1), _ndps(0) {}


// private
DistanceMetricCalculatorType::DistanceMetricCalculatorType( int lA, int lB, bool b)
    : _id(-1), _bilat(b), _lA(lA), _lB(lB), _edv( -1, lA, lB, b), _ndps(0)
{
}   // end ctor


QString DistanceMetricCalculatorType::params() const
{
    return (QString("%1").arg(_lA) + " " + QString("%1").arg(_lB));
}   // end params


MCTI::Ptr DistanceMetricCalculatorType::fromParams( const QString& prms) const
{
    std::istringstream iss(prms.toStdString());
    std::string cA, cB;
    iss >> cA;
    cB = cA;
    if ( iss.good())
        iss >> cB;

    if ( !LDMKS_MAN::hasCode(cA) || !LDMKS_MAN::hasCode(cB))
    {
        std::cerr << "[WARNING] FaceTools::Metric::DistanceMetricCalculatorType::fromParams: "
                  << "Landmarks " << cA << " and/or " << cB << " not found!" << std::endl;
        return nullptr;
    }   // end if

    FaceTools::Landmark::Landmark* lmkA = LDMKS_MAN::landmark(cA);
    FaceTools::Landmark::Landmark* lmkB = LDMKS_MAN::landmark(cB);

    // Non-bilateral landmarks CANNOT be the same since this denotes a point.
    if ( lmkA == lmkB && !lmkA->isBilateral() && !lmkB->isBilateral())
    {
        std::cerr << "[WARNING] FaceTools::Metric::DistanceMetricCalculatorType::fromParams: "
                  << "Cannot create interlandmark distance calculator for landmark if it's non-bilateral!" << std::endl;
        return nullptr;
    }   // end if

    // If lA and lB are the same, the measurement cannot be bilateral even if the landmarks are.
    const bool bilat = (lmkA != lmkB) && (lmkA->isBilateral() || lmkB->isBilateral());
    return MCTI::Ptr( new DistanceMetricCalculatorType( lmkA->id(), lmkB->id(), bilat));
}   // end fromParams


void DistanceMetricCalculatorType::setId( int id) { _id = id; _edv.setMetricId(_id);}
void DistanceMetricCalculatorType::setName( const QString& nm) { _name = nm;}
void DistanceMetricCalculatorType::setDescription( const QString& ds) { _desc = ds;}
void DistanceMetricCalculatorType::setNumDecimals( size_t ndps) { _ndps = ndps;}


bool DistanceMetricCalculatorType::canCalculate( const FM* fm) const
{
    LandmarkSet::Ptr lmks = fm->landmarks();
    return lmks->has(_lA) && lmks->has(_lB);
}   // end canCalculate


double DistanceMetricCalculatorType::measure( size_t, const FM* fm, FaceLateral lat) const
{
    assert( canCalculate(fm));
    LandmarkSet::Ptr lmks = fm->landmarks();

    FaceLateral latA = lat;
    FaceLateral latB = lat;

    if ( isBilateral())
    {
        if ( lat == FACE_LATERAL_MEDIAL)
        {
            std::cerr << "[ERROR] FaceTools::Metric::DistanceMetricCalculatorType::measure: Must specify left or right lateral for bilateral measure!" << std::endl;
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
            std::cerr << "[ERROR] FaceTools::Metric::DistanceMetricCalculatorType::measure: Cannot specify face lateral for non-bilateral measure!" << std::endl;
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
