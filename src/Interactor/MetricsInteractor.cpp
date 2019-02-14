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

#include <MetricsInteractor.h>
#include <MetricCalculatorManager.h>
#include <MetricVisualiser.h>
#include <ModelSelector.h>
#include <FaceModel.h>
#include <FaceView.h>
using FaceTools::Interactor::MetricsInteractor;
using FaceTools::Interactor::MEEI;
using FaceTools::Vis::MetricVisualiser;
using FaceTools::Vis::FV;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using FaceTools::Metric::MC;
using FaceTools::Action::ModelSelector;


MetricsInteractor::MetricsInteractor( const MEEI* meei)
    : ModelViewerInteractor( nullptr, nullptr)
{
    connect( meei, &MEEI::onEnterProp, this, &MetricsInteractor::doOnEnterProp);
    setEnabled(false);
}   // end ctor


void MetricsInteractor::doOnEnterProp( const FV* fv, const vtkProp* p)
{
    MetricVisualiser* vis = qobject_cast<MetricVisualiser*>( fv->layer(p));
    if ( vis && fv == ModelSelector::selected())
    {
        setEnabled(false);  // Prevent interactor from responding to a possible change in prop
        //MCM::setActiveMetric( vis->metricId());
        setEnabled(true);
    }   // end if
}   // end doOnEnterProp

