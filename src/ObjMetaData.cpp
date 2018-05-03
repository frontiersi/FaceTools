
bool ObjMetaData::makeBoundaryHandles( const std::list<int>& boundary, std::vector<cv::Vec3f>& bhandles) const
{
    if ( boundary.empty())
        return false;

    cv::Vec3f nvec, uvec;
    if ( !getOrientation( nvec, uvec))
        return false;

    cv::Vec3f mp(0,0,0);
    BOOST_FOREACH ( int vidx, boundary)
        mp += _model->vtx(vidx);
    mp *= 1.0f/boundary.size();    // Middle of boundary

    // Go round the loop again checking to see which vertices are at 12 extremes
    const cv::Vec3f rvec = uvec.cross(nvec);
    std::vector<cv::Vec3f> vvs(12);
    static const double THRD = 1./3;
    vvs[0] = uvec;
    cv::normalize(  2*THRD*uvec +   THRD*rvec, vvs[1]);
    cv::normalize(    THRD*uvec + 2*THRD*rvec, vvs[2]);
    vvs[3] = rvec;
    cv::normalize(  2*THRD*rvec -   THRD*uvec, vvs[4]);
    cv::normalize(    THRD*rvec - 2*THRD*uvec, vvs[5]);
    vvs[6] = -uvec;
    cv::normalize( -2*THRD*uvec -   THRD*rvec, vvs[7]);
    cv::normalize(   -THRD*uvec - 2*THRD*rvec, vvs[8]);
    vvs[9] = -rvec;
    cv::normalize(    THRD*uvec - 2*THRD*rvec, vvs[10]);
    cv::normalize(  2*THRD*uvec -   THRD*rvec, vvs[11]);

    bhandles.resize(12);
    std::vector<double> vdps(12);   // Max dot-products in the corresponding directions
    BOOST_FOREACH ( int vidx, boundary)
    {
        const cv::Vec3f& v = _model->vtx(vidx);
        const cv::Vec3d dv = v-mp;
        for ( int i = 0; i < 12; ++i)
        {
            const double dp = dv.dot(vvs[i]);
            if ( dp > vdps[i])
            {
                vdps[i] = dp;
                bhandles[i] = v;
            }   // end if
        }   // end for
    }   // end foreach
    return true;
}   // makeBoundaryHandles


const RFeatures::ObjModelCurvatureMap::Ptr ObjMetaData::rebuildCurvatureMap( int svidx)
{
    const IntSet& sfids = _model->getFaceIds( svidx);
    _curvMap = RFeatures::ObjModelCurvatureMap::create( _model, *sfids.begin());
}   // end rebuildCurvatureMap
