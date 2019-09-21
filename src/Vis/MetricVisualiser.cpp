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

#include <Vis/MetricVisualiser.h>
#include <Metric/MetricCalculatorManager.h>
#include <Metric/MetricValue.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <QDebug>
#include <sstream>
#include <iomanip>
using FaceTools::Vis::MetricVisualiser;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FM;
using FaceTools::Action::Event;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using MC = FaceTools::Metric::MetricCalculator;


MetricVisualiser::MetricVisualiser( int id) : _id(id)
{}   // end ctor


MetricVisualiser::~MetricVisualiser()
{
    while (!_fvs.empty())
        purge( const_cast<FV*>(_fvs.first()), Event::NONE);
}   // end dtor


void MetricVisualiser::apply( FV* fv, const QPoint*)
{
    assert(fv);
    purge(fv, Event::NONE);
    _fvs.insert(fv);
    doApply(fv);
    setVisible(fv, true);
}   // end apply


bool MetricVisualiser::purge( FV* fv, Event)
{
    _fvs.erase(fv);
    doPurge(fv);
    return true;
}   // end purge
