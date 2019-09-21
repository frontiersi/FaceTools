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

#include <Metric/GrowthData.h>
#include <Metric/MetricCalculatorManager.h>
#include <Ethnicities.h>
#include <FaceModel.h>
#include <QSet>
using FaceTools::Metric::GrowthData;
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MetricValue;
using MCM = FaceTools::Metric::MetricCalculatorManager;


GrowthData::Ptr GrowthData::create( int mid, size_t ndims, int8_t sex, int ethn)
{
    return Ptr( new GrowthData( mid, ndims, sex, ethn), [](GrowthData* x){ delete x;});
}   // end create


// private
GrowthData::GrowthData( int mid, size_t ndims, int8_t sex, int ethn)
    : _mid(mid), _sex(sex), _ethn(ethn), _n(0), _rsds(ndims)
{
}   // end ctor


void GrowthData::setSource( const QString& s)
{
    _source = s;
    if ( !_source.isEmpty() && !_source.endsWith("."))
        _source += ".";
}   // end setSource


void GrowthData::addSource( const QString& s)
{
    QString esrc = _source;

    // Remove the period at the end if it exists.
    while ( esrc.endsWith("."))
        esrc = esrc.left(esrc.size()-1);

    // Split into ; delimited tokens
    QStringList srcs = esrc.split(";");

    // Set the new set of references (trimmed)
    QSet<QString> sset;
    for ( const QString& es : srcs)
        sset.insert(es.trimmed());

    // Add the new source
    QString ns = s;
    while ( ns.endsWith("."))
        ns = ns.left(ns.size()-1);
    sset.insert(ns.trimmed());

    // Sort them
    srcs = QStringList::fromSet(sset);
    srcs.sort();

    // Finally, reset.
    _source = QString( "%1.").arg(srcs.join("; "));
}   // end addSource


void GrowthData::setNote( const QString& s)
{
    _note = s;
    if ( !_note.isEmpty() && !_note.endsWith("."))
        _note += ".";
}   // end setNote


void GrowthData::appendNote( const QString& s)
{
    QStringList notes;
    notes << _note;
    notes << s;
    _note = notes.join(" ").trimmed();
    if ( !_note.isEmpty() && !_note.endsWith("."))
        _note += ".";
}   // end appendNote


void GrowthData::setLongNote( const QString& s)
{
    _lnote = s;
    if ( !_lnote.isEmpty() && !_lnote.endsWith("."))
        _lnote += ".";
}   // end setLongNote


namespace  {

void createAgeRange( std::vector<double>& trng, double v, double tmax)
{
    trng.clear();
    while ( v < tmax)
    {
        trng.push_back(v);
        v += 1.0;
    }   // end while
    trng.push_back(tmax);
}   // end createAgeRange

}   // end namespace


GrowthData::Ptr GrowthData::create( const std::vector<GrowthData::CPtr>& gds)
{
    const size_t ngds = gds.size();
    assert( ngds > 1);

    const GrowthData::CPtr& g0 = gds.front();

    const size_t nd = g0->dims();
    const int mid = g0->metricId();

    int8_t sex = UNKNOWN_SEX;
    int nsmps = 0;

    QStringList srcs, notes;
    QStringSet srcsSet, notesSet;

    for ( size_t i = 0; i < ngds; ++i)
    {
        const GrowthData::CPtr& gi = gds.at(i);
        assert( nd == gi->dims());
        assert( mid == gi->metricId());
        sex |= gi->sex();   // Sex can be different

        // There may be some growth data that don't have the number of samples given.
        // In this case, combining them with data that do have N given will lead to
        // a misrepresentative N so it's better to set N as unknown for the combination.
        if ( gi->n() == 0)
            nsmps = -1; // Denotes unknown number
        if ( nsmps >= 0)
            nsmps += gi->n();

        // Compile notes from different sources.
        if ( notesSet.count(gi->note()) == 0)
        {
            notes << gi->note();
            notesSet.insert( gi->note());
        }   // end if

        // Compile sources (if different).
        if ( srcsSet.count(gi->source()) == 0)
        {
            QString lsrc = gi->source();
            while ( lsrc.endsWith("."))
                lsrc = lsrc.left(lsrc.size()-1);
            srcs << lsrc;
            srcsSet.insert(gi->source());
        }   // end if
    }   // end for

    // Get the mixed ethnicity if necessary
    IntSet eset;
    for ( GrowthData::CPtr g : gds)
        eset.insert(g->ethnicity());
    int ethn = Ethnicities::codeMix( eset); // Look for an existing viable mixture
    if ( ethn == 0) // None found so make a new mixture
    {
        ethn = Ethnicities::makeMixedCode( eset);
        assert( ethn != 0);
    }   // end if

    // Create the combination GrowthData object
    GrowthData::Ptr gc = create( mid, nd, sex, ethn);
    srcs.sort();
    gc->setSource( QString( "%1.").arg(srcs.join("; ")));
    gc->setNote( notes.join(" ").trimmed());
    gc->setN( std::max( 0, nsmps));
    // Combine the distributions over each dimension
    std::vector<rlib::RSD::Ptr>& nrsds = gc->_rsds;
    nrsds.resize(nd);

    for ( size_t d = 0; d < nd; ++d)
    {
        double tmin = -DBL_MAX;
        double tmax = DBL_MAX;
        std::vector<rlib::RSD::CPtr> drsds(ngds);   // Want the average over dimension d
        for ( size_t i = 0; i < ngds; ++i)
        {
            drsds[i] = gds.at(i)->_rsds.at(d);
            // Get the min and max range for the independent variable on this dimension.
            // Note that the combined age domain is the intersection of the age domains over all.
            tmin = std::max( tmin, drsds[i]->tmin());
            tmax = std::min( tmax, drsds[i]->tmax());
        }   // end for
        std::vector<double> trng;
        createAgeRange( trng, tmin, tmax);
        nrsds[d] = rlib::RSD::average( trng, drsds);
    }   // end for

    return gc;
}   // end create


// private
GrowthData::~GrowthData(){}



bool GrowthData::isWithinAgeRange( double age) const
{
    bool inRng = age > 0.0;
    if ( inRng)
    {
        const size_t dm = dims();
        for ( size_t i = 0; i < dm; ++i)
        {
            if ( age < rsd(i)->tmin() || age > rsd(i)->tmax())
            {
                inRng = false;
                break;
            }   // end if
        }   // end for
    }   // end if
    return inRng;
}   // end isWithinAgeRange
