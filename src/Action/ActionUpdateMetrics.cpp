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

#include <ActionUpdateMetrics.h>
#include <MetricCalculatorManager.h>
#include <PhenotypeManager.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <QDebug>
#include <algorithm>
using FaceTools::Action::ActionUpdateMetrics;
using FaceTools::Action::ModelSelector;
using FaceTools::Action::FaceAction;
using FaceTools::Metric::MC;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using FaceTools::Metric::PhenotypeManager;
using FaceTools::Metric::Phenotype;
using FaceTools::Metric::MC;


ActionUpdateMetrics::ActionUpdateMetrics()
{
    setVisible( false);
    setRespondToEvent( GEOMETRY_CHANGE);
    setRespondToEvent( LANDMARKS_ADD);
    setRespondToEvent( LANDMARKS_CHANGE);
    setRespondToEvent( METADATA_CHANGE);
    setRespondToEvent( SURFACE_DATA_CHANGE);

    for ( MC::Ptr mc : MCM::metrics())
        connect( &*mc, &MC::updated, this, &ActionUpdateMetrics::doUpdateMetric);
}   // end ctor


// Ready if at least one of the metrics can be calculated
bool ActionUpdateMetrics::testReady( const FV* fv)
{
    for ( MC::Ptr mc : MCM::metrics())
    {
        if ( mc->canCalculate(fv->data()))
            return true;
    }   // end for
    return false;
}   // end testReady


bool ActionUpdateMetrics::doAction( FVS& fvs, const QPoint&)
{
    if ( !ModelSelector::selected())
        return false;

    qDebug( " --- Acquiring metrics and testing phenotypes (demographics ignored) ---");

    // Recalculate all metrics
    FM *fm = ModelSelector::selected()->data();
    fm->clearPhenotypes();

    for ( MC::Ptr mc : MCM::metrics())
    {
        if ( mc->canCalculate(fm))
            mc->calculate(fm);
    }   // end for

    for ( int pid : PhenotypeManager::ids())
    {
        Phenotype::Ptr pt = PhenotypeManager::phenotype(pid);
        if ( pt->isPresent( &fm->cmetrics(), &fm->cmetricsL(), &fm->cmetricsR()))
            fm->addPhenotype(pid);
    }   // end for

    fvs.clear();
    fvs.insert( fm);
    return true;
}   // end doAction


void ActionUpdateMetrics::doUpdateMetric( int mid)
{
    if ( !updateMetric(mid))
        return;

    EventSet cset;
    cset.insert( METRICS_CHANGE);
    FVS fvs;
    fvs.insert(ModelSelector::selected());
    emit reportFinished( cset, fvs, true);
}   // end doUpdateMetric


bool ActionUpdateMetrics::updateMetric( int mid)
{
    if ( !ModelSelector::selected())
        return false;

    FM *fm = ModelSelector::selected()->data();
    MC::Ptr mc = MCM::metric(mid);
    if ( mc->canCalculate(fm))
    {
        mc->calculate(fm);

        // Retest presence of associated phenotypes
        for ( int pid : PhenotypeManager::metricPhenotypeIds(mid))
        {
            fm->removePhenotype(pid);
            Phenotype::Ptr pt = PhenotypeManager::phenotype(pid);
            if ( pt->isPresent( &fm->cmetrics(), &fm->cmetricsL(), &fm->cmetricsR()))
                fm->addPhenotype(pid);
        }   // end for
    }   // end if
    return true;
}   // end updateMetric
