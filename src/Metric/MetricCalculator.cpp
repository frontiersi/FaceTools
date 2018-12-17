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
#include <MiscFunctions.h>
#include <QtCharts/QSplineSeries>
#include <cassert>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <sol.hpp>
using FaceTools::Metric::MetricCalculator;
using FaceTools::Metric::MetricValue;
using FaceTools::Metric::DimensionStat;
using FaceTools::Metric::GrowthData;
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MCTI;
using FaceTools::FM;
using FaceTools::FaceLateral;


namespace {
static const QStringSet EMPTY_QSTRING_SET;
}   // end namespace

MetricCalculator::Ptr MetricCalculator::create( MCTI::Ptr mcti)
{
    return Ptr( new MetricCalculator( mcti), [](MetricCalculator* d){ delete d;});
}   // end create


MetricCalculator::Ptr MetricCalculator::load( const QString &fpath)
{
    sol::state lua;
    lua.open_libraries( sol::lib::base);

    MetricCalculator::Ptr mc;
    try
    {
        lua.script_file( fpath.toStdString());
        mc = Ptr( new MetricCalculator, [](MetricCalculator* d){ delete d;});
    }   // end try
    catch ( const sol::error& e)
    {
        std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: Unable to load and execute file '" << fpath.toStdString() << "'!" << std::endl;
        std::cerr << "\t" << e.what() << std::endl;
    }   // end catch

    if ( !mc)
        return nullptr;

    auto table = lua["mc"];
    if ( !table.valid())
    {
        std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: Lua file has no global member named mc!" << std::endl;
        return nullptr;
    }   // end if

    if ( !table["stats"].valid())
    {
        std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: stats member is not a table!" << std::endl;
        return nullptr;
    }   // end if

    int id = table["id"].get_or(-1);
    int ndps = table["ndps"].get_or(-1);
    int ndims = table["dims"].get_or(0);

    QString name, desc, catg, prms;
    if ( sol::optional<std::string> v = table["name"]) name = v.value().c_str();
    if ( sol::optional<std::string> v = table["desc"]) desc = v.value().c_str();
    if ( sol::optional<std::string> v = table["catg"]) catg = v.value().c_str();
    if ( sol::optional<std::string> v = table["prms"]) prms = v.value().c_str();

    if ( id < 0 || name.isEmpty() || ndps < 0 || catg.isEmpty() || prms.isEmpty() || ndims <= 0)
    {
        std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: incomplete metric metadata!" << std::endl;
        return nullptr;
    }   // end if

    MCTI::Ptr mcti = MetricCalculatorTypeRegistry::createFrom( catg, prms);
    if ( !mcti)
    {
        std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: Invalid category or parameters for metric " << id << ":" << std::endl;
        std::cerr << "\t" << catg.toStdString() << "; " << prms.toStdString() << std::endl;
        return nullptr;
    }   // end if

    mc->setType(mcti);
    mcti->setId(id);
    mcti->setName(name);
    mcti->setDescription(desc);
    mcti->setNumDecimals(static_cast<size_t>(ndps));

    // Read in the growth data
    sol::table stats = table["stats"];
    for ( size_t i = 1; i <= stats.size(); ++i)   // for each distribution i in the stats table
    {
        if ( !stats[i].valid())
        {
            std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: stats member is not a table for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        sol::table dist = stats[i];

        QString ethn, sexs, srcs, note;
        int nsmp = dist["nsmp"].get_or(0);  // Number of sample points
        if ( sol::optional<std::string> v = dist["ethn"]) ethn = v.value().c_str();
        if ( sol::optional<std::string> v = dist["sexs"]) sexs = v.value().c_str();
        if ( sol::optional<std::string> v = dist["srce"]) srcs = v.value().c_str();
        if ( sol::optional<std::string> v = dist["note"]) note = v.value().c_str();

        if ( ethn.isEmpty())
        {
            std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: distribution ethnicity not specified for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        if ( sexs.isEmpty())
        {
            std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: distribution sexs not specified for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        if ( srcs.isEmpty())
        {
            std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: distribution source not specified for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        if ( !dist["data"].valid())
        {
            std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: Distribution data member is not a table for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        sol::table data = dist["data"];
        const int tdims = int( data.size());
        if ( tdims != ndims)
        {
            std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: number of dimensions mismatch in growth data for metric " << id << "!" << std::endl;
            continue;
        }   // end if

        // Collect the distribution data
        std::vector<rlib::RSD::Ptr> rsds(static_cast<size_t>(tdims));
        for ( int j = 1; j <= tdims; ++j)
        {
            if ( !data[j].valid())
            {
                std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: data dimension is not a table for metric " << id << "!" << std::endl;
                continue;
            }   // end if

            // Collect the data points for this dimension
            Vec_3DP dvec;
            sol::table dimj = data[j];
            for ( size_t k = 1; k <= dimj.size(); ++k)
            {
                if ( !dimj[k].valid())
                {
                    std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: distribution dimension datapoint is not a table for metric " << id << "!" << std::endl;
                    continue;
                }   // end if

                sol::table dp = dimj[k];
                if ( dp.size() != 3 || !dp[1].valid() || !dp[2].valid() || !dp[3].valid())
                {
                    std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::load: distribution dimension datapoints must be numberic 3-tuples for metric " << id << "!" << std::endl;
                    continue;
                }   // end if

                double t = dp[1];
                double y = dp[2];
                double z = dp[3];
                dvec.push_back( {t,y,z});
            }   // end for

            rsds[static_cast<size_t>(j-1)] = rlib::RSD::create(dvec);
        }   // end for

        GrowthData* gd = new GrowthData( static_cast<size_t>(ndims));
        gd->setEthnicity(ethn);
        gd->setSex( fromSexString( sexs));
        gd->setSource( srcs);
        gd->setNote( note);
        gd->setN( nsmp);
        for ( size_t j = 0; j < static_cast<size_t>(ndims); ++j)
        {
            if ( rsds.at(j))
                gd->setRSD( j, rsds.at(j));
        }   // end for
        mc->addGrowthData(gd);
    }   // end for

    return mc;
}   // end load


MetricCalculator::MetricCalculator( MCTI::Ptr mcti)
    : _mcti(mcti), _visible(false) {}

MetricCalculator::MetricCalculator()
    : _mcti(nullptr), _visible(false) {}


MetricCalculator::~MetricCalculator()
{
    for ( GrowthData* gd : _agd) delete gd;
    _gdata.clear();
    _agd.clear();
}   // end dtor


const GrowthData* MetricCalculator::growthData( const QString& ethn, int8_t sex) const
{
    if ( _gdata.empty())
        return nullptr;

    const MetricGrowthData* mgd = &_gdata.at(_csrc);
    assert(mgd);

    // Use the defult ethnicity unless the specified one is available.
    const std::unordered_map<int8_t, GrowthData*> *sdata = &mgd->at(_deth);
    const QString lethn = ethn.toLower();
    if ( mgd->count(lethn) > 0)
        sdata = &mgd->at(lethn);

    if ( sdata->count( sex) == 0)
        return nullptr;

    return sdata->at( sex);
}   // end growthData


const GrowthData* MetricCalculator::growthData( const FM* fm) const
{
    const QString& ethn = fm->ethnicity();
    const GrowthData* gd = growthData( ethn, FEMALE_SEX | MALE_SEX);
    if ( !gd)
        gd = growthData( ethn, fm->sex());
    return gd;
}   // end growthData


bool MetricCalculator::canCalculate( const FM* fm) const
{
    return _mcti->canCalculate(fm);
}   // end canCalculate


void MetricCalculator::addGrowthData( GrowthData* gd)
{
    const QString src = gd->source();
    const QString eth = gd->ethnicity().toLower();
    _gdata[src][eth][gd->sex()] = gd;
    _agd.insert(gd);
    _ethnicities[src].insert(gd->ethnicity());
    _sources.insert(src);
    if ( _csrc.isEmpty())
        _csrc = src;
    if ( _deth.isEmpty())
        _deth = eth;
}   // end addGrowthData


// private
MetricValue MetricCalculator::calcMetricValue( const FM* fm, FaceLateral faceLat) const
{
    MetricValue mv( id());

    const GrowthData* gd = growthData(fm);
    if ( gd)
    {
        mv.setEthnicity( gd->ethnicity().toStdString());
        mv.setSource( gd->source().toStdString());
        mv.setSex( gd->sex());
    }   // end if

    for ( size_t i = 0; i < dims(); ++i)
    {
        DimensionStat dstat( _mcti->measure( i, fm, faceLat)); // Facial measurement at dimension i
        if ( gd)
        {
            rlib::RSD::Ptr rsd = gd->rsd(i);
            assert(rsd);
            const double t = std::min<double>( fm->age(), int(rsd->tmax() + 0.5));
            dstat.eage = t;
            dstat.mean = rsd->mval( t);   // Mean at t
            dstat.stdv = rsd->zval( t);   // Standard deviation at t
        }   // end if

        mv.addStat(dstat);
    }   // end for

    return mv;
}   // end calcMetricValue


bool MetricCalculator::setSource( const QString& src)
{
    if ( !src.isEmpty() && _sources.count(src) == 0)
    {
        std::cerr << "[WARNING] FaceTools::Metric::MetricCalculator::setSource: requested source \"" << src.toStdString() << "\" is not available!" << std::endl;
        return false;
    }   // end if
    _csrc = src;
    return true;
}   // end setSource


const QStringSet& MetricCalculator::ethnicities() const
{
    if ( _ethnicities.empty())
        return EMPTY_QSTRING_SET;
    return _ethnicities.at(_csrc);
}   // end ethnicities


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


double MetricCalculator::addSeriesToChart( QtCharts::QChart *chart, const GrowthData* gdata, double *xmin, double *xmax) const
{
    assert(gdata);
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

        const rlib::RSD::Ptr rsd = gdata->rsd(d);
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
