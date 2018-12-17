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
#include <algorithm>
using FaceTools::Action::ActionUpdateMetrics;
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
    std::cerr << " --- Recalculating metrics and testing for phenotype presence (demographics ignored) ---" << std::endl;

    FMS fms = fvs.models();   // Copy out
    for ( FM* fm : fms)
    {
        // Recalculate all metrics
        for ( MC::Ptr mc : MCM::metrics())
        {
            if ( mc->canCalculate(fm))
                mc->calculate(fm);
        }   // end for

        // Test presence of phenotypes
        fm->clearPhenotypes();
        for ( int hid : PhenotypeManager::ids())
        {
            Phenotype::Ptr hpo = PhenotypeManager::phenotype(hid);
            if ( hpo->isPresent( &fm->metrics(), &fm->metricsL(), &fm->metricsR()))
                fm->addPhenotype(hid);
        }   // end for
    }   // end for

    fvs.clear();
    fvs.insert(fms);

    return true;
}   // end doAction
