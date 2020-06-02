/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <Detect/LandmarksUpdater.h>
#include <LndMrk/LandmarksManager.h>
#include <MiscFunctions.h>
#include <FaceTools.h>
#include <FaceModel.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cassert>

using FaceTools::Detect::LandmarksUpdater;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::FM;
using FaceTools::Vec3f;
using FaceTools::FaceLateral;


dlib::shape_predictor LandmarksUpdater::s_shapePredictor;  // static initialisation

namespace {

/*
void drawDots( cv::Mat& dimg, const std::vector<cv::Point2f>& dots)
{
    const int nrows = dimg.rows - 1;
    const int ncols = dimg.cols - 1;
    for ( const cv::Point2f& p : dots)
    {
        cv::Point pt( p.x * ncols, p.y * nrows);
        cv::line( dimg, pt, pt, CV_RGB(255,255,255), 3);
    }   // end foreach
}   // end drawDots
*/


Vec3f toS( const FM *fm, const Vec3f &v) { return FaceTools::toSurface( fm->kdtree(), v);}
Vec3f toT( const FM *fm, const Vec3f &s, const Vec3f &t) { return FaceTools::toTarget( fm->kdtree(), s, t);}
Vec3f toD( const FM *fm, const Vec3f &u, const Vec3f &v) { return FaceTools::findHighOrLowPoint( fm->kdtree(), u, v);}


class LandmarkSetter
{
public:
    LandmarkSetter( const FM *fm, const QString& code, LandmarkSet &lms, const IntSet &ulms)
        : _fm(fm), _lmid( FaceTools::Landmark::LandmarksManager::codeId(code)), _lms(lms), _ulms(ulms)
    {
        _getl = _getr = [](){ return Vec3f(0,0,0);};
    }   // end ctor

    void setFnL( const std::function<Vec3f()> &fn) { _getl = fn;}
    void setFnR( const std::function<Vec3f()> &fn) { _getr = fn;}

    bool update()
    {
        bool modified = false;
        // Only update if the landmark set doesn't already have the landmark, or if it's specified to be modified.
        if ( !_lms.has( _lmid) || _ulms.count( _lmid) > 0)
        {
            modified = true;
            const Vec3f p0 = _getl();
            if ( FaceTools::Landmark::LandmarksManager::isBilateral( _lmid))
            {
                const Vec3f p1 = _getr();
                _lms.set( _lmid, p0, FaceLateral::FACE_LATERAL_LEFT);
                _lms.set( _lmid, p1, FaceLateral::FACE_LATERAL_RIGHT);
            }   // end if
            else
                _lms.set( _lmid, p0, FaceLateral::FACE_LATERAL_MEDIAL);
        }   // end if
        return modified;
    }   // update

private:
    const FM *_fm;
    int _lmid;
    LandmarkSet &_lms;
    const IntSet &_ulms;
    std::function<Vec3f()> _getl;
    std::function<Vec3f()> _getr;
};  // end class

}   // end namespace


Vec3f LandmarksUpdater::_toLocal( float x, float y, float z) const { return x*_xvec + y*_yvec + z*_zvec;}


bool LandmarksUpdater::_setBilateral( const QString& code, const Vec3f& v0, const Vec3f& v1, LandmarkSet &lms, const IntSet &ulms) const
{
    LandmarkSetter ls( _fm, code, lms, ulms);
    ls.setFnL( [&v0]() { return v0;});
    ls.setFnR( [&v1]() { return v1;});
    return ls.update();
}   // end _setBilateral


bool LandmarksUpdater::_setSingle( const QString& code, const Vec3f& v, LandmarkSet &lms, const IntSet &ulms) const
{
    LandmarkSetter ls( _fm, code, lms, ulms);
    ls.setFnL( [&v]() { return v;});
    return ls.update();
}   // end _setSingle


// Left and right palpebral superius [PS] (projected from 2D detection)
bool LandmarksUpdater::updatePS( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f lps = 0.5f*(vpts[37] + vpts[38]);
    const Vec3f rps = 0.5f*(vpts[43] + vpts[44]);
    // Pull initial projection up and back
    const Vec3f v0 = toT( _fm, lps, lps + _toLocal(0,5,-1));
    const Vec3f v1 = toT( _fm, rps, rps + _toLocal(0,5,-1));
    return _setBilateral( Landmark::PS, v0, v1, lms, ulms);
}   // end updatePS


// Left and right palpebral inferius [PI] (projected from 2D detection)
bool LandmarksUpdater::updatePI( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f lpi = 0.5f * (vpts[40] + vpts[41]);
    const Vec3f rpi = 0.5f * (vpts[46] + vpts[47]);
    // Pull initial projection back
    const Vec3f v0 = toT( _fm, lpi, lpi + _toLocal(0,0,-3));
    const Vec3f v1 = toT( _fm, rpi, rpi + _toLocal(0,0,-3));
    return _setBilateral( Landmark::PI, v0, v1, lms, ulms);
}   // end updatePI


// Left and right endocanthi [EN] (projecting plus small adjustment)
bool LandmarksUpdater::updateEN( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f v0 = toS( _fm, vpts[39] + _toLocal(  3, -2, -25)); // Bring left endocanthion inward slightly
    const Vec3f v1 = toS( _fm, vpts[42] + _toLocal( -3, -2, -25)); // Bring right endocanthion inward slightly
    return _setBilateral( Landmark::EN, v0, v1, lms, ulms);
}   // end updateEN


// Left and right exocanthi [EX] (projected plus small adjustment)
bool LandmarksUpdater::updateEX( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f EXW = _toLocal( 0, 0, -5);
    const Vec3f v0 = toS( _fm, vpts[36] + EXW);
    const Vec3f v1 = toS( _fm, vpts[45] + EXW);
    return _setBilateral( Landmark::EX, v0, v1, lms, ulms);
}   // end updateEX


// Pupils [P] as simple average of eye points
bool LandmarksUpdater::updateP( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f v0 = toS( _fm, 0.25f * (lms.pos( Landmark::EN, FACE_LATERAL_LEFT) +
                                            lms.pos( Landmark::EX, FACE_LATERAL_LEFT) +
                                            lms.pos( Landmark::PS, FACE_LATERAL_LEFT) +
                                            lms.pos( Landmark::PI, FACE_LATERAL_LEFT)));
    const Vec3f v1 = toS( _fm, 0.25f * (lms.pos( Landmark::EN, FACE_LATERAL_RIGHT) +
                                            lms.pos( Landmark::EX, FACE_LATERAL_RIGHT) +
                                            lms.pos( Landmark::PS, FACE_LATERAL_RIGHT) +
                                            lms.pos( Landmark::PI, FACE_LATERAL_RIGHT)));
    return _setBilateral( Landmark::P, v0, v1, lms, ulms);
}   // end updateP


namespace {

// Put two points on the surface - one above and one below the detected MSO point with distances
// half that to the pupil and find the deepest point which should be on the brow ridge in a line
// that contains the original detected MSO point.
Vec3f findMSO( const FM *fm, Vec3f v, const Vec3f& p)
{
    const Vec3f p2v = v-p;
    const Vec3f tb = toS( fm, p + 0.5f*p2v);   // Bottom
    const Vec3f tt = toS( fm, tb + p2v);       // Top
    return toD( fm, tb, tt);
}   // end findMSO

}   // end namespace


// Mid-supraorbital [MSO]
bool LandmarksUpdater::updateMSO( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    // [17,21] left brow left to right, [22,26] right brow left to right
    const Vec3f lmso = findMSO( _fm, (1.0f/3) * (vpts[18] + vpts[19] + vpts[20]), lms.pos( Landmark::P, FACE_LATERAL_LEFT));
    const Vec3f rmso = findMSO( _fm, (1.0f/3) * (vpts[23] + vpts[24] + vpts[25]), lms.pos( Landmark::P, FACE_LATERAL_RIGHT));
    const Vec3f v0 = toT( _fm, lmso, lmso + _toLocal( -5, -3, 30));
    const Vec3f v1 = toT( _fm, rmso, rmso + _toLocal(  5, -3, 30));
    return _setBilateral( Landmark::MSO, v0, v1, lms, ulms);
}   // end updateMSO


// Pronasale [PRN] as 2D detected nose tip but pulled outwards to peak
bool LandmarksUpdater::updatePRN( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f v = toT( _fm, vpts[30], vpts[30] + 50*_zvec);
    return _setSingle( Landmark::PRN, v, lms, ulms);
}   // end updatePRN


// Estimate sellion [SE] - deepest part of the nose bridge between the pupils (*** WILL UPDATE ***)
bool LandmarksUpdater::updateSE( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f v = toS( _fm, toD( _fm, lms.pos( Landmark::P, FACE_LATERAL_LEFT), lms.pos( Landmark::P, FACE_LATERAL_RIGHT)));
    return _setSingle( Landmark::SE, v, lms, ulms);
}   // end updateSE


// Estimate mid-nasal dorsum [MND] as initially halfway between sellion and pronasale
bool LandmarksUpdater::updateMND( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f v = toS( _fm, 0.5f * ( lms.pos( Landmark::SE) + lms.pos( Landmark::PRN)));
    return _setSingle( Landmark::MND, v, lms, ulms);
}   // end updateMND


// Glabella [G]
bool LandmarksUpdater::updateG( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f lmso = lms.pos( Landmark::MSO, FACE_LATERAL_LEFT);
    const Vec3f rmso = lms.pos( Landmark::MSO, FACE_LATERAL_RIGHT);
    const Vec3f lps = lms.pos( Landmark::PS, FACE_LATERAL_LEFT);
    const Vec3f rps = lms.pos( Landmark::PS, FACE_LATERAL_RIGHT);

    Vec3f v = toS( _fm, 0.5f * (lmso + rmso));
    const Vec3f tmp = toS( _fm, 0.25f * (lps + rps + lmso + rmso)); // Temp point halfway between palpebral superius and mso points
    // glabella as maximally off curve point between tmp and tmp point placed above halfway (x) between mso points
    v = toD( _fm, tmp, toS( _fm, 2*v - tmp));

    return _setSingle( Landmark::G, v, lms, ulms);
}   // end updateG


// Update sellion [SE] as deepest point on plane between mid-nasal dorsum and glabella
bool LandmarksUpdater::updateSE2( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f v = toD( _fm, lms.pos( Landmark::G), lms.pos( Landmark::MND));
    return _setSingle( Landmark::SE, v, lms, ulms);
}   // end updateSE2


// Update mid-nasal dorsum [MND] given new sellion position
bool LandmarksUpdater::updateMND2( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f v = toS( _fm, 0.5f * ( lms.pos( Landmark::SE) + lms.pos( Landmark::PRN)));
    return _setSingle( Landmark::MND, v, lms, ulms);
}   // end updateMND2


// Nasion [N]
bool LandmarksUpdater::updateN( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    // vpts[27] is nasal root, but not defined as either sellion or nasion so is not used as is.
    // Nasion is placed at height of superior palpebral sulcus in line with sellion.
    const Vec3f v = toD( _fm, lms.pos( Landmark::G), lms.pos( Landmark::SE));
    return _setSingle( Landmark::N, v, lms, ulms);
}   // end updateN


// Maxillofrontale [MF] left and right
bool LandmarksUpdater::updateMF( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f len = lms.pos(Landmark::EN, FACE_LATERAL_LEFT);
    const Vec3f ren = lms.pos(Landmark::EN, FACE_LATERAL_RIGHT);
    const Vec3f g = lms.pos(Landmark::G);
    const Vec3f mnd = lms.pos(Landmark::MND);

    Vec3f lref0 = toS( _fm, 0.5f * (g + len));
    Vec3f lref1 = toS( _fm, 0.5f * (mnd + len));
    const Vec3f lmf = toD( _fm, lref0, lref1);

    Vec3f rref0 = toS( _fm, 0.5f * (g + ren));
    Vec3f rref1 = toS( _fm, 0.5f * (mnd + ren));
    const Vec3f rmf = toD( _fm, rref0, rref1);

    return _setBilateral( Landmark::MF, lmf, rmf, lms, ulms);
}   // end updateMF


// Crista philtri [CPH]
bool LandmarksUpdater::updateCPH( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f lcph = toS( _fm, vpts[50] + _toLocal(-3,-1,8));    // Left (pull out and to left
    const Vec3f rcph = toS( _fm, vpts[52] + _toLocal( 3,-1,8));   // Right (pull out and to right)
    return _setBilateral( Landmark::CPH, lcph, rcph, lms, ulms);
}   // end updateCPH


// Subnasale [SN] as deepest point between pronasale and mean CPH (was vpts[33])
bool LandmarksUpdater::updateSN( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f mcph = 0.5f*(lms.pos( Landmark::CPH, FACE_LATERAL_LEFT) + lms.pos( Landmark::CPH, FACE_LATERAL_RIGHT));
    const Vec3f sn = toD( _fm, lms.pos( Landmark::PRN), mcph);
    return _setSingle( Landmark::SN, sn, lms, ulms);
}   // end updateSN


// Point at base of nostip between pronasale and subnasale.
Vec3f LandmarksUpdater::_nosetipBase( const LandmarkSet& lms) const
{
    const Vec3f prn = lms.pos( Landmark::PRN);
    const Vec3f sn = lms.pos( Landmark::SN);
    const Vec3f vec = sn-prn;
    const Vec3f ovec( vec[0], vec[2], -vec[1]);
    return toS( _fm, 0.5f*(prn + sn) + 0.5f*ovec);
}   // end _nosetipBase


// Alare [AL] surface projected detected points but pulled up and away from the face at 45 degrees.
bool LandmarksUpdater::updateAL( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    //const Vec3f lv = vpts[31];
    //const Vec3f rv = vpts[35];

    const Vec3f lp = lms.pos( Landmark::P, FACE_LATERAL_LEFT);
    const Vec3f rp = lms.pos( Landmark::P, FACE_LATERAL_RIGHT);
    const Vec3f ln = 0.5f*(lms.pos( Landmark::EN, FACE_LATERAL_LEFT) + lp);
    const Vec3f rn = 0.5f*(lms.pos( Landmark::EN, FACE_LATERAL_RIGHT) + rp);
    const Vec3f ntb = _nosetipBase( lms);

    // Define two points that by reference to the underneath of the nosetip and the inner canthi should
    // be a point close to the outer edge of the alare (on both sides).
    const Vec3f lvS = toS( _fm, ntb + (ln - ntb).dot(_xvec)*_xvec); // Point on left cheek under left endocanthian in horizontal line with ntb
    const Vec3f rvS = toS( _fm, ntb + (rn - ntb).dot(_xvec)*_xvec); // Same on right

    // Starting at these reference points on the surface, pull them out and forward...
    const Vec3f lvT = lvS + _toLocal(-10, 0, 2);
    const Vec3f rvT = rvS + _toLocal( 10, 0, 2);

    const Vec3f v0 = toT( _fm, lvS, lvT);
    const Vec3f v1 = toT( _fm, rvS, rvT);

    return _setBilateral( Landmark::AL, v0, v1, lms, ulms);
}   // end updateAL


// Alare curvature points [AC]
bool LandmarksUpdater::updateAC( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f lp = lms.pos( Landmark::P, FACE_LATERAL_LEFT);  // Left pupil
    const Vec3f rp = lms.pos( Landmark::P, FACE_LATERAL_RIGHT); // Right pupil
    const Vec3f lal = lms.pos( Landmark::AL, FACE_LATERAL_LEFT);
    const Vec3f ral = lms.pos( Landmark::AL, FACE_LATERAL_RIGHT);

    // Create reference points on the surface of the face either side of
    // the face under the pupils and in horizontal alignment with the alare.
    Vec3f lt = toS( _fm, lal + (lp - lal).dot(_xvec)*_xvec + (lp - lal).dot(_zvec)*_zvec);
    Vec3f rt = toS( _fm, ral + (rp - ral).dot(_xvec)*_xvec + (rp - ral).dot(_zvec)*_zvec);

    lt = toD( _fm, lt, lal);    // near Left AC
    rt = toD( _fm, rt, ral);    // near Right AC

    // Pull in and a little back
    const Vec3f v0 = toT( _fm, lt, lt + _toLocal( 5,0,-2));
    const Vec3f v1 = toT( _fm, rt, rt + _toLocal(-5,0,-2));

    return _setBilateral( Landmark::AC, v0, v1, lms, ulms);
}   // end updateAC


// Subalare [SBAL] (was using surface projected vpts[32] left and vpts[34] right)
bool LandmarksUpdater::updateSBAL( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    // Left and right alare
    const Vec3f lal = lms.pos( Landmark::AL, FACE_LATERAL_LEFT);
    const Vec3f ral = lms.pos( Landmark::AL, FACE_LATERAL_RIGHT);

    // Left and right alare curvature points
    const Vec3f lac = lms.pos( Landmark::AC, FACE_LATERAL_LEFT);
    const Vec3f rac = lms.pos( Landmark::AC, FACE_LATERAL_RIGHT);

    // Left and right crista philtri
    const Vec3f lcph = lms.pos( Landmark::CPH, FACE_LATERAL_LEFT);
    const Vec3f rcph = lms.pos( Landmark::CPH, FACE_LATERAL_RIGHT);

    // Subnasale
    const Vec3f sn = lms.pos( Landmark::SN);

    // Get outer reference points at horizontal level of subnasale and in vertical alignment with AC on the surface.
    //const Vec3f l0 = toS( _fm, sn + (lal - sn).dot(_xvec)*_xvec + (lac - sn).dot(_zvec)*_zvec);
    //const Vec3f r0 = toS( _fm, sn + (ral - sn).dot(_xvec)*_xvec + (rac - sn).dot(_zvec)*_zvec);

    const Vec3f l0 = toS( _fm, 0.25f * (lcph + sn + lal + lac));
    const Vec3f r0 = toS( _fm, 0.25f * (rcph + sn + ral + rac));
    
    // Target to pull towards
    const Vec3f t = (0.0f*lac + 1.0f*sn + 0.0f*rac) - 0.5f*(lac - rac).norm()*_zvec;

    const Vec3f v0 = toT( _fm, l0, t);
    const Vec3f v1 = toT( _fm, r0, t);

    return _setBilateral( Landmark::SBAL, v0, v1, lms, ulms);
}   // end updateSBAL


// Cheilion [CH]
bool LandmarksUpdater::updateCH( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    Vec3f v0 = vpts[48];   // Left - push in and back a little
    Vec3f v1 = vpts[54];   // Right - push in and back a little
    v0 = toT( _fm, v0, v0 + _toLocal( 2, 6, -5));
    v1 = toT( _fm, v1, v1 + _toLocal(-2, 6, -5));
    return _setBilateral( Landmark::CH, v0, v1, lms, ulms);
}   // updateCH


// Stomion superius [STS]
bool LandmarksUpdater::updateSTS( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    //Vec3f sts = toS( _fm, 1.0f/3 * (vpts[65] + vpts[66] + vpts[67]));
    //Vec3f sti = toS( _fm, 1.0f/3 * (vpts[61] + vpts[62] + vpts[63]));
    // Detector can confuse placement of stomion inferius/superius, so check relative heights.
    //if ( _yvec.dot(sts) < _yvec.dot(sti))
    //      std::swap( sts, sti);
    const Vec3f lch = lms.pos( Landmark::CH, FACE_LATERAL_LEFT);
    const Vec3f rch = lms.pos( Landmark::CH, FACE_LATERAL_RIGHT);
    const Vec3f lcph = lms.pos( Landmark::CPH, FACE_LATERAL_LEFT);
    const Vec3f rcph = lms.pos( Landmark::CPH, FACE_LATERAL_RIGHT);
    const Vec3f mv = 0.25f*(lch + rch + lcph + rcph);  // Mean point
    const Vec3f v = toS( _fm, mv);

    return _setSingle( Landmark::STS, v, lms, ulms);
}   // end updateSTS


// Labiale inferius [LI] (was using mean surface projected vpts[{56,57,58}])
bool LandmarksUpdater::updateLI( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f lch = lms.pos( Landmark::CH, FACE_LATERAL_LEFT);
    const Vec3f rch = lms.pos( Landmark::CH, FACE_LATERAL_RIGHT);
    const Vec3f p = 1.0f/3 * (vpts[56] + vpts[57] + vpts[58]);  // 2D projection
    const Vec3f mv = (1.0f/3)*(p + lch + rch);
    Vec3f v = toS( _fm, mv);    // Should be somewhere on middle of bottom lip
    v = toT( _fm, v, v + _toLocal(0,-15,50)); // Pull out and down a little
    return _setSingle( Landmark::LI, v, lms, ulms);
}   // end updateLI


// Stomion inferius [STI]
bool LandmarksUpdater::updateSTI( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    //Vec3f sts = toS( _fm, 1.0f/3 * (vpts[65] + vpts[66] + vpts[67]));
    //Vec3f sti = toS( _fm, 1.0f/3 * (vpts[61] + vpts[62] + vpts[63]));
    // Detector can confuse placement of stomion inferius/superius, so check relative heights.
    //if ( _yvec.dot(sts) < _yvec.dot(sti))
    //    std::swap( sts, sti);
    const Vec3f lch = lms.pos( Landmark::CH, FACE_LATERAL_LEFT);
    const Vec3f rch = lms.pos( Landmark::CH, FACE_LATERAL_RIGHT);
    const Vec3f li = lms.pos( Landmark::LI);
    const Vec3f mv = (1.0f/3)*(li + lch + rch);
    const Vec3f sts = lms.pos( Landmark::STS);
    const Vec3f v = toT( _fm, mv, sts); // Pull toward STS
    return _setSingle( Landmark::STI, v, lms, ulms);
}   // end updateSTI


// Labiale superius [LS] (was using surface projected vpts[51])
bool LandmarksUpdater::updateLS( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f v = toD( _fm, lms.pos( Landmark::STS), lms.pos( Landmark::SN));
    // TODO - fit angle to saddle by minimising angle surface normal makes with _zvec
    return _setSingle( Landmark::LS, v, lms, ulms);
}   // end updateLS


// Pogonion [PG]
bool LandmarksUpdater::updatePG( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f sn = lms.pos( Landmark::SN);
    const Vec3f sti = lms.pos( Landmark::STI);
    const Vec3f pg = toS( _fm, sn + 3*(sti - sn));
    const Vec3f v = toT( _fm, pg, pg + _toLocal(0,-20,40));   // Pull out and a little down
    return _setSingle( Landmark::PG, v, lms, ulms);
}   // end updatePG



// Sublabiale [SL]
bool LandmarksUpdater::updateSL( const std::vector<Vec3f>& vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    const Vec3f lch = lms.pos( Landmark::CH, FACE_LATERAL_LEFT);
    const Vec3f rch = lms.pos( Landmark::CH, FACE_LATERAL_RIGHT);
    const float mw = static_cast<float>((lch - rch).norm());   // Mouth width
    Vec3f li = lms.pos( Landmark::LI);
    li = toD( _fm, li, toS( _fm, li - 0.5f*mw*_yvec));
    const Vec3f v = toT( _fm, li, li + _toLocal( 0, 2, -10));    // Push in and up a little
    return _setSingle( Landmark::SL, v, lms, ulms);
}   // end updateSL


void LandmarksUpdater::_setLandmarks( const std::vector<Vec3f> &vpts, LandmarkSet &lms, const IntSet &ulms) const
{
    updatePS(   vpts, lms, ulms);
    updatePI(   vpts, lms, ulms);
    updateEN(   vpts, lms, ulms);
    updateEX(   vpts, lms, ulms);
    updateP(    vpts, lms, ulms);
    updateMSO(  vpts, lms, ulms);
    updatePRN(  vpts, lms, ulms);
    updateSE(   vpts, lms, ulms);
    updateMND(  vpts, lms, ulms);
    updateG(    vpts, lms, ulms);
    updateSE2(  vpts, lms, ulms);
    updateMND2( vpts, lms, ulms);
    updateN(    vpts, lms, ulms);
    updateMF(   vpts, lms, ulms);
    updateCPH(  vpts, lms, ulms);
    updateSN(   vpts, lms, ulms);
    updateAL(   vpts, lms, ulms);
    updateAC(   vpts, lms, ulms);
    updateSBAL( vpts, lms, ulms);
    updateCH(   vpts, lms, ulms);
    updateSTS(  vpts, lms, ulms);
    updateLI(   vpts, lms, ulms);
    updateSTI(  vpts, lms, ulms);
    updateLS(   vpts, lms, ulms);
    updatePG(   vpts, lms, ulms);
    updateSL(   vpts, lms, ulms);

    //lms.set( L_UPP_VERM, vpts[49]
    //lms.set( R_UPP_VERM, vpts[53]
    //lms.set( LOW_LIP_B_0, vpts[55]
    //lms.set( LOW_LIP_B_4, vpts[59]

    // MOUTH OPENING
    //lms.set( MOUTH_OPEN_0, vpts[60]
    //lms.set( MOUTH_OPEN_4, vpts[64]
}   // end _setLandmarks


// public static
bool LandmarksUpdater::initialise( const std::string& fdat)
{
    bool success = false;
    try
    {
        dlib::deserialize( fdat) >> s_shapePredictor;
        success = s_shapePredictor.num_parts() > 0;
    }   // end try
    catch ( const dlib::serialization_error&)
    {
        std::cerr << "[ERROR] FaceTools::Detect::LandmarksUpdater::initialise: Deserialisation error using \""
                  << fdat << "\"" << std::endl;
        success = false;
    }   // end catch

    return success;
}   // end initialise


// public static
bool LandmarksUpdater::isinit() { return s_shapePredictor.num_parts() > 0;}


LandmarksUpdater::LandmarksUpdater( const r3dvis::OffscreenMeshViewer &vwr, const FM *fm)
    : _vwr(vwr), _fm(fm), _xvec(1,0,0), _yvec(0,1,0), _zvec(0,0,1) {}


void LandmarksUpdater::setOrientation( const Vec3f &nvec, const Vec3f &uvec)
{
    _zvec = nvec;
    _zvec.normalize();
    _yvec = uvec;
    _yvec.normalize();
    _xvec = uvec.cross(nvec);
    _xvec.normalize();
}   // end setOrientation


bool LandmarksUpdater::detect( LandmarkSet &lmks, const IntSet &ulmks) const
{
    if ( s_shapePredictor.num_parts() == 0)
    {
        std::cerr << "[ERROR] FaceTools::Detect::LandmarksUpdater::detect: Not initialised!" << std::endl;
        return -1;
    }   // end if

    cv::Mat_<cv::Vec3b> map = _vwr.snapshot();
    const int nrows = map.rows;
    const int ncols = map.cols;

    dlib::cv_image<dlib::bgr_pixel> img(map);

    dlib::frontal_face_detector faceDetector( dlib::get_frontal_face_detector());
    std::vector<dlib::rectangle> dets = faceDetector( img);
    if ( dets.empty())
    {
        std::cerr << "[WARNING] FaceTools::Detect::LandmarksUpdater::detect: dlib::frontal_face_detector failed to detect landmarks!" << std::endl;
        return -1;
    }   // end if

    size_t j = 0;
    long bottom = dets[0].bottom();
    // Only want the lowest detection in the set
    for ( size_t i = 1; i < dets.size(); ++i)
    {
        if ( dets[i].bottom() > bottom)
        {
            j = i;
            bottom = dets[i].bottom();
        }   // end if
    }   // end else

    dlib::full_object_detection det = s_shapePredictor( img, dets[j]);
    const size_t numParts = det.num_parts();
    int nfound = 0;

    std::vector<bool> foundVec( numParts);
    std::vector<cv::Point2f> cpts( numParts);
    for ( size_t i = 0; i < numParts; ++i)
    {
        const dlib::point& pt = det.part(i);
        foundVec[i] = pt != dlib::OBJECT_PART_NOT_PRESENT;
        if ( !foundVec[i])
            cpts[i] = cv::Point2f(0,0);
        else
        {
            cpts[i] = cv::Point2f( float(pt.x() + 0.5f)/ncols, float(pt.y() + 0.5f)/nrows);
            nfound++;
        }   // end else
    }   // end for

    bool updated = false;
    if ( nfound == 68)
    {
        updated = true;
        std::vector<Vec3f> vpts(nfound);
        // Vertices < 17 ignored since these are boundary vertices and are not accurate.
        for ( int i = 17; i < nfound; ++i)
            vpts[i] = foundVec[i] ? _vwr.worldPosition( cpts[i]) : Vec3f(0,0,0);
        _setLandmarks( vpts, lmks, ulmks);
    }   // end if

    return updated;
}   // end detect
