/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <FileIO/FaceModelFileData.h>
#include <LndMrk/LandmarksManager.h>
#include <Metric/PhenotypeManager.h>
#include <Metric/MetricManager.h>
#include <QTemporaryDir>

using FaceTools::FileIO::FaceModelFileData;
using FaceTools::FileIO::Content;
using FaceTools::Metric::MetricValue;
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::PhenotypeManager;
using FaceTools::Metric::Phenotype;
using FaceTools::FaceAssessment;
using FaceTools::FaceSide;
using FaceTools::Vec3f;
using FaceTools::FM;

using LMAN = FaceTools::Landmark::LandmarksManager;
using MM = FaceTools::Metric::MetricManager;


Content FaceTools::FileIO::operator|( const Content &c0, const Content &c1)
{
    return Content( uint8_t(c0) | uint8_t(c1));
}   // end operator|


Content FaceTools::FileIO::operator&( const Content &c0, const Content &c1)
{
    return Content( uint8_t(c0) & uint8_t(c1));
}   // end operator&


struct FaceModelFileData::HPO
{
    std::string id, name, metrics;  // metrics is comma separated list of metric IDs
    bool ageMatch, sexMatch;
    bool mEthMatch, pEthMatch;
};  // end struct


namespace {
const PTree *getAssessment( const PTree &fnode, const std::string &namedAssessor="")
{
    const PTree &anodes = fnode.get_child("Assessments");
    const int defaultId = anodes.get<int>("DefaultAssessment");
    const PTree *tass = nullptr;
    const PTree *dass = nullptr;

    for ( const PTree::value_type &vnode : anodes)
    {
        const PTree &anode = vnode.second;
        if ( anode.count("AssessmentId") == 0)
            continue;   // Skip no content nodes

        if ( !namedAssessor.empty() && anode.get<std::string>("Assessor") == namedAssessor)
            tass = &anode;

        if ( anode.get<int>("AssessmentId") == defaultId)
            dass = &anode;
    }   // end for

    if ( !tass)
        tass = dass;

    return tass;
}   // end getAssessment


MetricSet readMetricSet( const PTree &msetNode)
{
    MetricSet mset;

    for ( const PTree::value_type &mnode : msetNode)
    {
        const PTree &mv = mnode.second; // MetricValue node
        if ( mv.count("id") == 0)   // Skip no content node
            continue;

        const int mid = mv.get<int>( "id");
        const size_t ndims = mv.get<size_t>( "ndims");
        const bool planar = mv.get<bool>( "planar");
        std::vector<float> dvals( ndims);

        const PTree &stats = mv.get_child("stats");
        for ( const PTree::value_type &snode : stats)
        {
            const PTree &dv = snode.second; // Dimension node
            if ( dv.count("axis") == 0) // Skip no content node
                continue;

            const int d = int(dv.get<size_t>( "axis"));
            dvals[d] = dv.get<float>( "value");
        }   // end for

        mset.set( MetricValue( mid, nullptr, dvals, planar));
    }   // end for

    return mset;
}   // end readMetricSet


std::string str2csv( const QString& v)
{
    return ("\"" + v.simplified().replace("\"","\"\"") + "\"").toStdString();
}   // end str2csv


std::string str2csv( const std::string& v) { return str2csv( QString::fromStdString(v));}

}   // end namespace


std::list<int> FaceModelFileData::_sortedMeasurementIds() const
{
    std::list<int> mids;
    for ( int mid : MM::ids())
        if ( hasMeasurement(mid))
            mids.push_back(mid);
    mids.sort();
    return mids;
}   // end _sortedMeasurementIds


void FaceModelFileData::_clearHPOTerms()
{
    for ( HPO *hpo : _hpos)
        delete hpo;
    _hpos.clear();
}   // end _clearHPOTerms


void FaceModelFileData::_readHPOTerms( const PTree &hposNode)
{
    for ( const PTree::value_type &hnode : hposNode)
    {
        const PTree &hv = hnode.second; // Term node
        if ( hv.count("ID") == 0)
            continue;   // Skip no content node

        HPO *hpo = new HPO;
        hpo->id = hv.get<std::string>( "ID");
        hpo->name = hv.get<std::string>( "Name");
        hpo->metrics = hv.get<std::string>( "Metrics");
        hpo->ageMatch = hv.get<bool>( "AgeMatch");
        hpo->sexMatch = hv.get<bool>( "SexMatch");
        hpo->mEthMatch = hv.get<bool>( "MaternalEthnicityMatch");
        hpo->pEthMatch = hv.get<bool>( "PaternalEthnicityMatch");
        _hpos.push_back(hpo);
    }   // end for
}   // end _readHPOTerms


FaceModelFileData::~FaceModelFileData()
{
    _clearHPOTerms();
}   // end dtor


FaceModelFileData::FaceModelFileData( const FM &fm)
    : _fm( &fm)
{
    const IntSet hids = PhenotypeManager::discover( fm, fm.currentAssessment()->id());
    for ( int hid : hids)
    {
        const Phenotype::Ptr p = PhenotypeManager::phenotype(hid);
        HPO *hpo = new HPO;
        hpo->id = PhenotypeManager::formattedId(hid).toStdString();
        hpo->name = p->name().toStdString();
        hpo->metrics = p->metricsList().toStdString();
        hpo->ageMatch = p->isAgeMatch( fm);
        hpo->sexMatch = p->isSexMatch( fm);
        hpo->mEthMatch = p->isMaternalEthnicityMatch( fm);
        hpo->pEthMatch = p->isPaternalEthnicityMatch( fm);
        _hpos.push_back(hpo);
    }   // end for
}   // end ctor


FaceModelFileData::FaceModelFileData( const QString &fpath, const QString &assessorName)
    : _fm( &_ifm)
{
    QTemporaryDir tdir;
    PTree ptree;
    _err = readMeta( fpath, tdir, ptree);
    double fversion = 0.0;
    if ( !_err.isEmpty() || !importMetaData( _ifm, ptree, fversion))
        return;

    if ( fversion < 5.0)
    {
        _err = QObject::tr("File version must be 5.0 or greater!");
        return;
    }   // end if

    // Set FM assessment to the req. assessor (for landmarks, paths, etc).
    FaceAssessment& ass = _setCurrentAssessment( assessorName);

    // Read in the measurements for the selected assessor.
    const PTree &fnode = ptree.get_child("faces").get_child("FaceModels").get_child("FaceModel");
    const PTree *assm = getAssessment( fnode, assessorName.toStdString());
    assert( assm);
    if ( assm->count("MetricGroups") > 0)
    {
        const PTree &mgroups = assm->get_child("MetricGroups");
        ass.metrics(RIGHT) = readMetricSet( mgroups.get_child("RightLateral"));
        ass.metrics(LEFT) = readMetricSet( mgroups.get_child("LeftLateral"));
        ass.metrics(MID)  = readMetricSet( mgroups.get_child("Frontal"));
    }   // end if

    if ( assm->count("HPO_Terms") > 0)
        _readHPOTerms( assm->get_child("HPO_Terms"));
}   // end ctor


FaceAssessment& FaceModelFileData::_setCurrentAssessment( const QString &nm)
{
    for ( int aid : _ifm.assessmentIds())
    {
        const FaceAssessment::CPtr ass = _ifm.assessment(aid);
        if ( ass->assessor() == nm)
        {
            _ifm.setCurrentAssessment( aid);
            break;
        }   // end if
    }   // end for
    return *_ifm.currentAssessment();
}   // end _setCurrentAssessment


float FaceModelFileData::measurementValue( int mid, FaceSide lat, size_t d) const
{
    assert( hasMeasurement( mid));
    const MetricSet &mset = _fm->currentAssessment()->cmetrics(lat);
    const MetricValue &mv = mset.metric( mid);
    assert( d >= 0 && d < mv.ndims());
    return mv.value( d);
}   // end measurementValue


bool FaceModelFileData::measurementInPlane( int mid, FaceSide lat) const
{
    assert( hasMeasurement( mid));
    const MetricSet &mset = _fm->currentAssessment()->cmetrics(lat);
    return mset.metric(mid).planar();
}   // end measurementInPlane


bool FaceModelFileData::hasMeasurement( int mid) const { return _fm->currentAssessment()->hasMetric(mid);}


void FaceModelFileData::_printSummaryLine( std::ostream &os, Content content) const
{
    os << str2csv( imageId()) << "," << str2csv( subjectId());

    if ( (content & Content::SEX) != Content::EMPTY)
        os << "," << sex().toStdString();

    if ( (content & Content::AGE) != Content::EMPTY)
    {
        os << "," << dateOfBirth().toString().toStdString() << "," << captureDate().toString().toStdString() << ","
           << std::fixed << std::setprecision(2) << age();
    }   // end if

    if ( (content & Content::ETHNICITY) != Content::EMPTY)
        os << "," << str2csv( maternalEthnicityString()) << "," << str2csv( paternalEthnicityString());

    if ( (content & Content::IMAGE) != Content::EMPTY)
        os << "," << str2csv( studyId()) << "," << str2csv( source()) << "," << str2csv( assessor()) << "," << str2csv( notes());

    os << std::endl;    // Basic info all on first line
}   // end _printSummaryLine


// Second line details row/line/record counts for subsequent sections
void FaceModelFileData::_printSectionLine( std::ostream &os, Content content) const
{
    size_t lcount = 0;
    if ( !landmarks().empty() && (content & Content::LANDMARKS) != Content::EMPTY)
        lcount = 2*(LMAN::ids().size() - LMAN::medialIds().size()) + LMAN::medialIds().size();

    size_t mcount = 0;
    if ( (content & Content::MEASUREMENTS) != Content::EMPTY)
    {
        const std::list<int> mids = _sortedMeasurementIds();
        for ( int mid : mids)
            mcount += MM::bilateralIds().count(mid) + 1;
    }   // end if

    size_t pcount = 0;
    if ( (content & Content::PATHS) != Content::EMPTY)
        pcount = paths().ids().size();

    size_t hcount = 0;
    if ( (content & Content::HPOS) != Content::EMPTY)
        hcount = _hpos.size();

    os << lcount << "," << mcount << "," << pcount << "," << hcount << std::endl;
}   // end _printSectionLine


void FaceModelFileData::toCSV( std::ostream &os, Content content) const
{
    if ( content == Content::EMPTY) // Set all content to export if none given
    {
        content = Content::SEX |
                  Content::AGE |
                  Content::ETHNICITY |
                  Content::IMAGE |
                  Content::LANDMARKS |
                  Content::MEASUREMENTS |
                  Content::PATHS |
                  Content::HPOS;
    }   // end if

    _printSummaryLine( os, content);
    _printSectionLine( os, content);

    if ( (content & Content::LANDMARKS) != Content::EMPTY)
        _landmarksToCSV( os);

    if ( (content & Content::MEASUREMENTS) != Content::EMPTY)
        _measurementsToCSV( os);

    if ( (content & Content::PATHS) != Content::EMPTY)
        _pathsToCSV( os);

    if ( (content & Content::HPOS) != Content::EMPTY)
        _hposToCSV( os);
}   // end toCSV


namespace {
char getLateralChar( FaceSide lat)
{
    if ( lat == FaceTools::LEFT)
        return 'L';
    if ( lat == FaceTools::RIGHT)
        return 'R';
    return 'M';
}   // end getLateralChar


std::ostream &printVec( const Vec3f& pos, std::ostream &os)
{
    os << std::fixed << std::setprecision(2) << pos[0]
       << "," << std::fixed << std::setprecision(2) << pos[1]
       << "," << std::fixed << std::setprecision(2) << pos[2];
    return os;
}   // end printVec

}   // end namespace


void FaceModelFileData::_printLandmark( int lid, FaceSide lat, const std::string &nm, std::ostream &os) const
{
    const Vec3f &pos = landmarks().pos( lid, lat);
    os << lid << "," << nm << "," << getLateralChar(lat) << ",";
    printVec( pos, os) << std::endl;
}   // end _printLandmark


void FaceModelFileData::_printMeasurement( int mid, FaceSide lat, const std::string &nm, size_t ndims, std::ostream &os) const
{
    const bool planar = measurementInPlane( mid, lat);
    os << mid << "," << nm << "," << getLateralChar( lat) << "," << std::boolalpha << planar << "," << ndims;
    for ( size_t i = 0; i < ndims; ++i)
        os << "," << std::fixed << std::setprecision(2) << measurementValue( mid, lat, i);
    os << std::endl;
}   // end _printMeasurement


void FaceModelFileData::_landmarksToCSV( std::ostream &os) const
{
    const Landmark::LandmarkSet &lmks = landmarks();
    std::list<int> lids( lmks.ids().begin(), lmks.ids().end());
    lids.sort();
    for ( int lid : lids)
    {
        const std::string nm = str2csv( LMAN::landmark(lid)->name());
        if ( !LMAN::isBilateral( lid))
            _printLandmark( lid, MID, nm, os);
        else
        {
            _printLandmark( lid, LEFT, nm, os);
            _printLandmark( lid, RIGHT, nm, os);
        }   // end else
    }   // end for
}   // end _landmarksToCSV


void FaceModelFileData::_measurementsToCSV( std::ostream &os) const
{
    const std::list<int> mids = _sortedMeasurementIds();
    for ( int mid : mids)
    {
        Metric::Metric::Ptr mc = MM::metric(mid);
        const std::string nm = str2csv( mc->name());
        const size_t ndims = mc->dims();

        if ( !mc->isBilateral())
            _printMeasurement( mid, MID, nm, ndims, os);
        else
        {
            _printMeasurement( mid, LEFT, nm, ndims, os);
            _printMeasurement( mid, RIGHT, nm, ndims, os);
        }   // end else
    }   // end for
}   // end _measurementsToCSV


void FaceModelFileData::_pathsToCSV( std::ostream &os) const
{
    std::list<int> pids( paths().ids().begin(), paths().ids().end());
    pids.sort();
    for ( int pid : pids)
    {
        const Path &path = paths().path(pid);
        os << pid << "," << str2csv(path.name()) << ",";
        printVec( path.handle0(), os) << ",";
        printVec( path.handle1(), os) << ",";
        printVec( path.depthHandle(), os) << ",";
        printVec( path.orientation(), os) << ",";
        os << std::fixed << std::setprecision(2) << path.euclideanDistance() << ","
           << std::fixed << std::setprecision(2) << path.surfaceDistance() << ","
           << std::fixed << std::setprecision(2) << path.depth() << ","
           << std::fixed << std::setprecision(2) << path.angleAtDepth() << ","
           << std::fixed << std::setprecision(2) << path.crossSectionalArea() << std::endl;
    }   // end for
}   // end _pathsToCSV


void FaceModelFileData::_hposToCSV( std::ostream &os) const
{
    for ( HPO* hpo : _hpos)
    {
        os << hpo->id << "," << str2csv(hpo->name) << "," << str2csv(hpo->metrics) << ","
           << std::boolalpha << hpo->ageMatch << "," << hpo->sexMatch << ","
           << hpo->mEthMatch << "," << hpo->pEthMatch << std::endl;
    }   // end for
}   // end _hposToCSV
