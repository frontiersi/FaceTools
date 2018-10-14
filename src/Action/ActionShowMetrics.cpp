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

#include <ActionShowMetrics.h>
#include <MetricCalculatorManager.h>
#include <MetricVisualiser.h>
#include <ModelSelector.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <algorithm>
using FaceTools::Action::ActionShowMetrics;
using FaceTools::Action::FaceAction;
using FaceTools::Interactor::MEEI;
using FaceTools::Interactor::MetricsInteractor;
using FaceTools::Metric::MCSet;
using FaceTools::Metric::MC;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;
using MCM = FaceTools::Metric::MetricCalculatorManager;


ActionShowMetrics::ActionShowMetrics( const QString& dn, const QIcon& ico, const MEEI* meei)
    : FaceAction( dn, ico), _interactor(meei)
{
    setCheckable( true, false);
    ProcessFlagPredicate pfp = [this](const FVS&){ return this->isChecked();};
    setRespondToEvent( METADATA_CHANGE, pfp);
    setRespondToEvent( GEOMETRY_CHANGE, pfp);
    setRespondToEvent( LANDMARKS_ADD, pfp);
    setRespondToEvent( LANDMARKS_CHANGE, pfp);

    connect( &_interactor, &MetricsInteractor::onEnterMetric, this, &ActionShowMetrics::doOnEnterMetric);
    connect( &_interactor, &MetricsInteractor::onLeaveMetric, this, &ActionShowMetrics::doOnLeaveMetric);

    for ( MC::Ptr mc : MCM::vmetrics())
        connect( &*mc, &MC::updated, this, &ActionShowMetrics::doOnMetricUpdated);
}   // end ctor


// private slot
void ActionShowMetrics::doOnMetricUpdated()
{
    MC* mc = qobject_cast<MC*>(sender());
    for ( FM* fm : _vmodels) // Update mc's visualiser on all currently visualised models.
    {
        const FVS& fvs = fm->fvs();
        std::for_each( std::begin(fvs), std::end(fvs), [=](FV* fv){ fv->remove(mc->visualiser());});
        if ( isChecked() && mc->isVisible())
            std::for_each( std::begin(fvs), std::end(fvs), [=](FV* fv){ fv->apply(mc->visualiser());});
    }   // end for
    FaceTools::updateRenderers( _vmodels);
}   // end doOnMetricUpdated


// private slot
void ActionShowMetrics::doOnEnterMetric( const FV* fv, int mid)
{
    MC::Ptr mc = MCM::metric(mid);
    mc->visualiser()->updateCaptions( fv->data());
    mc->visualiser()->setCaptionsVisible( true);
    fv->data()->updateRenderers();
    emit onEnterMetric( mid);
}   // end doOnEnterMetric


// private slot
void ActionShowMetrics::doOnLeaveMetric( const FV* fv, int mid)
{
    MC::Ptr mc = MCM::metric(mid);
    mc->visualiser()->setCaptionsVisible( false);
    fv->data()->updateRenderers();
}   // end doOnLeaveMetric


bool ActionShowMetrics::testIfCheck( const FV* fv) const
{
    if ( !fv)
        return false;
    for ( MC::Ptr mc : MCM::vmetrics())
    {
        if ( fv->isApplied( mc->visualiser()))
            return true;
    }   // end for
    return false;
}   // end testIfCheck


namespace {
bool canVisualiseAny( const FaceTools::Metric::MetricSet& mset)
{
    for ( int id : mset.ids())
    {
        if ( MCM::metric(id)->visualiser() != nullptr)
            return true;
    }   // end for
    return false;
}   // end canVisualiseAny
}   // end namespace


bool ActionShowMetrics::testReady( const FV* fv)
{
    assert(fv);
    const FM* fm = fv->data();
    // Ready only if the model has metrics set that can be visualised
    fm->lockForRead();
    const bool canShow = canVisualiseAny( fm->metrics())
                      || canVisualiseAny( fm->metricsL())
                      || canVisualiseAny( fm->metricsR());
    fm->unlock();
    return canShow;
}   // end testReady


bool ActionShowMetrics::doAction( FVS& fvsin, const QPoint&)
{
    assert(fvsin.size() == 1);
    FV* fv = fvsin.first();
    FM* fm = fv->data();
    purge( fm); // Remove all visualisers initially

    size_t numShownMetrics = 0;

    if ( isChecked())
    {
        _vmodels.insert(fm);
        const MCSet& mcs = MCM::vmetrics();
        for ( MC::Ptr mc : mcs)
        {
            if ( mc->isVisible())  // Skip if not selected to be visualised
            {
                numShownMetrics++;
                const FVS& fvs = fm->fvs();
                std::for_each( std::begin(fvs), std::end(fvs), [=](FV* fv){ fv->apply( mc->visualiser());});
            }   // end if
        }   // end for
    }   // end if

    // Won't stay checked if no metrics were shown!
    if ( isChecked() && numShownMetrics == 0)
        setChecked(false);

    // Prevent deselection of the selected view until no longer showing metrics
    // since repicking the model will be disabled.
    fv->setPickable(!isChecked());
    ModelSelector::setSelectEnabled( !isChecked());

    return true;
}   // end doAction


void ActionShowMetrics::purge( const FM* fm)
{
    const MCSet& mcs = MCM::vmetrics();
    const FVS& fvs = fm->fvs();
    for ( FV* fv : fvs)
        std::for_each( std::begin(mcs), std::end(mcs), [=](MC::Ptr mc){ fv->remove( mc->visualiser());});
    _vmodels.erase(const_cast<FM*>(fm));
}   // end purge
