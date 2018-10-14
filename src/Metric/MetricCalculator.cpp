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

#include <MetricCalculator.h>
#include <MetricCalculatorTypeRegistry.h>
#include <FaceModel.h>
#include <QtCharts/QSplineSeries>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <fstream>
using FaceTools::Metric::MetricCalculator;
using FaceTools::Metric::MetricValue;
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MCTI;
using FaceTools::FM;
using FaceTools::FaceLateral;


MetricCalculator::Ptr MetricCalculator::create( MCTI::Ptr mcti)
{
    return Ptr( new MetricCalculator( mcti), [](MetricCalculator* d){ delete d;});
}   // end create


MetricCalculator::Ptr MetricCalculator::fromFile( const std::string &fpath)
{
    MetricCalculator::Ptr mc;
    try
    {
        std::ifstream ifs;
        ifs.open( fpath);
        if ( !ifs.good())
        {
            ifs.close();
            return mc;
        }   // end if

        mc = Ptr( new MetricCalculator, [](MetricCalculator* d){ delete d;});
        ifs >> *mc;
        ifs.close();

        if ( mc->type() == nullptr)  // Failed to read
            mc = nullptr;
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        mc = nullptr;
    }   // end catch

    return mc;
}   // end fromFile


// private
MetricCalculator::MetricCalculator( MCTI::Ptr mcti)
    : _mcti(mcti), _visible(false) {}

MetricCalculator::MetricCalculator()
    : _mcti(nullptr), _visible(false) {}


// private
MetricValue MetricCalculator::calcMetricValue( const FM* fm, FaceLateral faceLat) const
{
    const size_t d = dims();
    std::vector<double> v(d);
    std::vector<double> z(d);
    std::vector<bool> zok(d);
    for ( size_t i = 0; i < d; ++i)
    {
        v[i] = _mcti->measure( i, fm, faceLat);
        rlib::RSD::Ptr rsd = _mcti->rsd(i);
        const double MAX_AGE = int(rsd->tmax() + 0.5);
        const double tage = std::min<double>( fm->age(), MAX_AGE);

        z[i] = DBL_MAX;
        zok[i] = false;
        if ( rsd)
        {
            z[i] = rsd->zscore( tage, v[i]);
            zok[i] = fabs( tage - fm->age()) < 0.00001;
        }   // end if
    }   // end for

    return MetricValue( id(), v, z, zok);
}   // end calcMetricValue


bool MetricCalculator::calculate( FM* fm) const
{
    using namespace FaceTools::Metric;
    if ( _mcti)
    {
        if ( _mcti->isBilateral())
        {
            fm->metricsL().set( calcMetricValue( fm, FACE_LATERAL_LEFT));
            fm->metricsR().set( calcMetricValue( fm, FACE_LATERAL_RIGHT));
        }   // end if
        else
            fm->metrics().set( calcMetricValue( fm, FACE_LATERAL_MEDIAL));
    }   // end if

    return _mcti != nullptr;
}   // end calculate


double MetricCalculator::addSeriesToChart( QtCharts::QChart *chart, double *xmin, double *xmax) const
{
    using namespace QtCharts;

    double x0 = DBL_MAX;
    double x1 = -DBL_MAX;

    const size_t ndims = _mcti->dims();
    for ( size_t d = 0; d < ndims; ++d)
    {
        QSplineSeries *mseries = new QSplineSeries;
        QSplineSeries *z1pseries = new QSplineSeries;
        QSplineSeries *z2pseries = new QSplineSeries;
        QSplineSeries *z1nseries = new QSplineSeries;
        QSplineSeries *z2nseries = new QSplineSeries;

        const rlib::RSD::Ptr rsd = _mcti->rsd(d);
        const int minx = int(rsd->tmin());
        const int maxx = int(rsd->tmax() + 0.5);

        for ( int i = minx; i <= maxx; ++i)
        {
            double a = i;
            double m = rsd->mval(a);
            double z = rsd->zval(a);
            mseries->append( a, m);
            z1pseries->append( a, m + z);
            z2pseries->append( a, m + 2*z);
            z1nseries->append( a, m - z);
            z2nseries->append( a, m - 2*z);
        }   // end for

        QString dsuff = ndims > 1 ? QString(" (D%1)").arg(d) : "";
        z2pseries->setName("+2SD" + dsuff);
        z1pseries->setName("+1SD" + dsuff);
        mseries->setName("Mean" + dsuff);
        z1nseries->setName("-1SD" + dsuff);
        z2nseries->setName("-2SD" + dsuff);

        QPen pen;
        pen.setWidth(1);
        pen.setStyle(Qt::DashLine);
        pen.setColor(Qt::red);
        z2pseries->setPen(pen);
        pen.setColor(Qt::darkRed);
        z1pseries->setPen(pen);

        pen.setColor(Qt::blue);
        z2nseries->setPen(pen);
        pen.setColor(Qt::darkBlue);
        z1nseries->setPen(pen);

        pen.setStyle(Qt::SolidLine);
        pen.setColor(Qt::black);
        mseries->setPen(pen);

        chart->addSeries(z2pseries);
        chart->addSeries(z1pseries);
        chart->addSeries(mseries);
        chart->addSeries(z1nseries);
        chart->addSeries(z2nseries);

        x0 = std::min<double>( x0, minx);
        x1 = std::max<double>( x1, maxx);
    }   // end for

    if ( xmin)
        *xmin = x0;
    if ( xmax)
        *xmax = x1;

    return x1 - x0;
}   // end addSeriesToChart


namespace {
const std::string ID_TAG   = "ID:";
const std::string NAME_TAG = "NAME:";
const std::string DESC_TAG = "DESC:";
const std::string NDPS_TAG = "NDPS:";
const std::string CATG_TAG = "CATG:";
const std::string PRMS_TAG = "PRMS:";
const std::string SRCE_TAG = "SRCE:";
const std::string ETHN_TAG = "ETHN:";
const std::string SEXS_TAG = "SEXS:";
const std::string DIMS_TAG = "DIMS:";


std::string getRmLine( std::istringstream& iss, bool lower=false)
{
    std::string ln;
    std::getline( iss, ln);
    boost::algorithm::trim(ln);
    if ( lower)
        boost::algorithm::to_lower(ln);
    return ln;
}   // end getRmLine


bool isSampleHeader( const std::string& tag, size_t didx)
{
    std::ostringstream oss;
    oss << "D_" << didx << ":";
    return tag == oss.str();
}   // end isSampleHeader

}   // end namespace


std::ostream& FaceTools::Metric::operator<<( std::ostream& os, const MetricCalculator& mc)
{
    MCTI::Ptr mcti = mc.type();
    os << ID_TAG   << " " << mcti->id() << std::endl;
    os << NAME_TAG << " " << mcti->name() << std::endl;
    os << DESC_TAG << " " << mcti->description() << std::endl;
    os << NDPS_TAG << " " << mcti->numDecimals() << std::endl;
    os << CATG_TAG << " " << mcti->category() << std::endl;
    os << PRMS_TAG << " " << mcti->params() << std::endl;
    os << SRCE_TAG << " " << mcti->source() << std::endl;
    os << ETHN_TAG << " " << mcti->ethnicities() << std::endl;
    os << SEXS_TAG << " " << toSexString( mcti->sex()) << std::endl;

    const size_t ndims = mcti->dims();
    os << DIMS_TAG << " " << ndims << std::endl << std::endl;   // Starts data section

    for ( size_t i = 0; i < ndims; ++i)
    {
        rlib::RSD::Ptr rsd = mcti->rsd(i);
        const Vec_3DP& dvec = rsd->data();
        os << "D_" << i << ": " << dvec.size() << std::endl;  // E.g. denote 17 samples making dimension 0 as "D_0: 17"

        /** Normal use of streaming syntax (below) failed to compile on G++ 5.4.0!
        using namespace rlib;
        os << dvec << std::endl; */
        // so forcing the use of the correct output stream operator function:
        rlib::operator<<( os, dvec);    // Write out the samples (with extra lnfd).
        os << std::endl;
    }   // end for
    return os;
}   // end operator<<



std::istream& FaceTools::Metric::operator>>( std::istream& is, MetricCalculator& mc)
{
    bool inHeader = true;
    size_t ndims = 0;      // Number of dimensions expected
    size_t didx = 0;       // Dimension index
    size_t nsamples = 0;   // Number of samples expected
    size_t sidx = 0;       // Sample index

    int ndps;
    int id;
    std::string ln, tag, dtag, name, desc, srce, ethn, cat, prms, emsg;
    FaceTools::Sex sex = FaceTools::UNKNOWN_SEX;
    DP t, y, z;
    Vec_3DP dvec;
    MCTI::Ptr mcti;

    while ( is.good() && !is.eof())
    {
        std::getline( is, ln);

        // Skip empty lines or lines starting with # (comments).
        if ( ln.empty() || ln[0] == '#')
            continue;

        std::istringstream iss(ln);

        if ( inHeader)
        {
            iss >> tag;
            if ( tag == ID_TAG)
                iss >> id;
            else if ( tag == NAME_TAG)
                name = getRmLine( iss);
            else if ( tag == DESC_TAG)
                desc = getRmLine(iss);
            else if ( tag == NDPS_TAG)
                iss >> ndps;
            else if ( tag == CATG_TAG)
                cat = getRmLine( iss, true);
            else if ( tag == PRMS_TAG)
                prms = getRmLine( iss, true);
            else if ( tag == SRCE_TAG)
                srce = getRmLine( iss, false);
            else if ( tag == ETHN_TAG)
                ethn = getRmLine( iss, true);
            else if ( tag == SEXS_TAG)
                sex = fromSexString( getRmLine( iss, true));
            else if ( tag == DIMS_TAG)
            {
                iss >> ndims;  // Read in the number of expected dimensions
                didx = 0;      // Next dimension to read in
                inHeader = false; // Ended header section

                if ( cat.empty() || prms.empty())
                {
                    emsg = "Reached end of header without metric category or parameters set!";
                    break;  // Skip remainder of file
                }   // end if
                else if ( (mcti = FaceTools::Metric::MetricCalculatorTypeRegistry::createFrom( cat, prms)) == nullptr)
                {
                    emsg = "Unable to create MetricCalculatorType from category " + cat + " with parameters " + prms;
                    break;  // Skip remainder of file
                }   // end else if
            }   // end else if
            else
            {
                emsg = "Unknown tag " + tag + " in header section!";
                break;  // Skip remainder of file
            }   // end else
        }   // end if
        else if ( didx < ndims)
        {
            if ( nsamples == 0)
            {
                iss >> tag;
                if ( isSampleHeader( tag, didx))
                {
                    iss >> nsamples;
                    sidx = 0;
                    dvec.clear();
                    if ( nsamples < 2)
                    {
                        emsg = "Too few samples!";
                        break;
                    }   // end else
                }   // end else if
            }   // end if
            else if ( sidx < nsamples)
            {
                t = y = z = 0;
                iss >> t >> y >> z;
                dvec.push_back( {t, y, z});
                sidx++;
            }   // end else

            if ( sidx == nsamples)
            {
                assert( nsamples > 0);

                rlib::RSD::Ptr rsd = rlib::RSD::create(dvec);
                mcti->setRSD( didx, rsd);

                nsamples = 0;
                didx++; // Next dimension to read
            }   // end else
        }   // end else if
        else
        {
            emsg = "Extra file content beyond specified number of metric dimensions.";
            break;
        }   // end else
    }   // end while

    if ( !emsg.empty())
        std::cerr << "[WARNING] operator>>( istream&, FaceTools::Metric::MetricCalculator): " << emsg << std::endl;
    else
    {
        mcti->setId( id);
        mcti->setName( name);
        mcti->setDescription( desc);
        mcti->setSource( srce);
        mcti->setEthnicities( ethn);
        mcti->setSex( sex);
        mcti->setNumDecimals( size_t(ndps < 0 ? 0 : ndps));
        mc.setType( mcti);
        mc.setVisible( mcti->visualiser() != nullptr);
    }   // end if

    return is;
}   // end operator>>
