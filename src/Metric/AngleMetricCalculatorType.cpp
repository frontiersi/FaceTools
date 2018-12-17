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

#include <AngleMetricCalculatorType.h>
#include <LandmarksManager.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <sstream>
using FaceTools::Metric::AngleMetricCalculatorType;
using FaceTools::Metric::MCTI;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::FM;
using FaceTools::FaceLateral;


const QString AngleMetricCalculatorType::s_cat("Angle");


AngleMetricCalculatorType::AngleMetricCalculatorType()
    : _id(-1), _bilat(false), _l0(-1), _lc(-1), _l1(-1), _vis( -1, -1, -1, -1), _ndps(0) {}


// private
AngleMetricCalculatorType::AngleMetricCalculatorType( int l0, int lc, int l1, bool b)
    : _id(-1), _bilat(b), _l0(l0), _lc(lc), _l1(l1), _vis( -1, l0, lc, l1, b), _ndps(0)
{
}   // end ctor


QString AngleMetricCalculatorType::params() const
{
    QString p;
    if ( _lc < 0)
        p = QString("%1 ").arg(_l0) + QString("%1").arg(_l1);
    else
        p = QString("%1 ").arg(_l0) + QString("%1 ").arg(_lc) + QString("%1").arg(_l1);
    return p;
}   // end params


MCTI::Ptr AngleMetricCalculatorType::fromParams( const QString& prms) const
{
    std::istringstream iss(prms.toStdString());
    std::string cA, cB, cC;
    iss >> cA;
    cB = cA;
    if ( iss.good())
    {
        iss >> cB;
        if ( iss.good())
            iss >> cC;
    }   // end if

    FaceTools::Landmark::Landmark* lmkC = nullptr;
    FaceTools::Landmark::Landmark* lmk0 = LDMKS_MAN::landmark(cA);
    FaceTools::Landmark::Landmark* lmk1 = LDMKS_MAN::landmark(cB);
    if ( !cC.empty())
    {
        lmkC = lmk1;
        lmk1 = LDMKS_MAN::landmark(cC);
    }   // end if

    if ( !lmk0 || !lmk1)
    {
        std::cerr << "[WARNING] FaceTools::Metric::AngleMetricCalculatorType::fromParams: "
                  << "Landmark endpoints not found!" << std::endl;
        return nullptr;
    }   // end if

    if ( !cC.empty() && !lmkC)
    {
        std::cerr << "[WARNING] FaceTools::Metric::AngleMetricCalculatorType::fromParams: "
                  << "Centre point landmark " << cC << " not found!" << std::endl;
        return nullptr;
    }   // end if

    // Non-bilateral landmarks CANNOT be the same since this denotes a point.
    if ( lmk0 == lmk1 && !lmk0->isBilateral() && !lmk1->isBilateral())
    {
        std::cerr << "[WARNING] FaceTools::Metric::AngleMetricCalculatorType::fromParams: "
                  << "Cannot create angle calculator for landmark if it's non-bilateral!" << std::endl;
        return nullptr;
    }   // end if

    if ( lmkC)
    {
        if ( lmk0 == lmkC || lmk1 == lmkC)
        {
            std::cerr << "[WARNING] FaceTools::Metric::AngleMetricCalculatorType::fromParams: "
                      << "Angle centre point cannot be the same as either of the endpoints!" << std::endl;
            return nullptr;
        }   // end if
    }   // end else

    // If lA and lB are the same, the measurement cannot be bilateral even if the landmarks are.
    const bool bilat = (lmk0 != lmk1) && (lmk0->isBilateral() || lmk1->isBilateral());
    int cid = lmkC ? lmkC->id() : -1;
    return MCTI::Ptr( new AngleMetricCalculatorType( lmk0->id(), cid, lmk1->id(), bilat));
}   // end fromParams


void AngleMetricCalculatorType::setId( int id) { _id = id; _vis.setMetricId(_id);}
void AngleMetricCalculatorType::setName( const QString& nm) { _name = nm;}
void AngleMetricCalculatorType::setDescription( const QString& ds) { _desc = ds;}
void AngleMetricCalculatorType::setNumDecimals( size_t ndps) { _ndps = ndps;}


bool AngleMetricCalculatorType::canCalculate( const FM* fm) const
{
    LandmarkSet::Ptr lmks = fm->landmarks();
    return lmks->has(_l0) && lmks->has(_l1) && (_lc < 0 || lmks->has(_lc));
}   // end canCalculate


double AngleMetricCalculatorType::measure( size_t, const FM* fm, FaceLateral lat) const
{
    assert( canCalculate(fm));
    LandmarkSet::Ptr lmks = fm->landmarks();

    FaceLateral latA = lat;
    FaceLateral latB = lat;
    FaceLateral latM = lat;

    if ( isBilateral())
    {
        if ( lat == FACE_LATERAL_MEDIAL)
        {
            std::cerr << "[ERROR] FaceTools::Metric::AngleMetricCalculatorType::measure: Must specify left or right lateral for bilateral measure!" << std::endl;
            assert(false);
        }   // end if

        latA = LDMKS_MAN::landmark(_l0)->isBilateral() ? lat : FACE_LATERAL_MEDIAL;
        latB = LDMKS_MAN::landmark(_l1)->isBilateral() ? lat : FACE_LATERAL_MEDIAL;
        latM = _lc >= 0 && LDMKS_MAN::landmark(_lc)->isBilateral() ? lat : FACE_LATERAL_MEDIAL;
        assert( latA != FACE_LATERAL_MEDIAL || latB != FACE_LATERAL_MEDIAL);    // Must be true of wouldn't be a bilateral measurement
    }   // end if
    else
    {
        // If not a bilateral measure, then either both landmarks must be medial,
        // or the landmarks must be the same.
        if ( lat != FACE_LATERAL_MEDIAL)
        {
            std::cerr << "[ERROR] FaceTools::Metric::AngleMetricCalculatorType::measure: Cannot specify face lateral for non-bilateral measure!" << std::endl;
            assert(false);
        }   // end if

        if ( _l0 == _l1)
        {
            latA = FACE_LATERAL_LEFT;
            latB = FACE_LATERAL_RIGHT;
        }   // end if
    }   // end else

    cv::Vec3d p0 = *lmks->pos(_l0, latA);
    cv::Vec3d p1 = *lmks->pos(_l1, latB);
    cv::Vec3d cp;
    if ( _lc >= 0)
        cp = *fm->landmarks()->pos(_lc, latM);
    else
    {
        const cv::Vec3f mp( 0.5 * (p0 + p1));
        cp = FaceTools::toSurface( fm->kdtree(), mp);
    }   // end else

    // Return a value in the range [0,pi]
    return 0.5 * CV_PI * (1.0 - RFeatures::cosi( cp, p0, p1));
}   // end measure
