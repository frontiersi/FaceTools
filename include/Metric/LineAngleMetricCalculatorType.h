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

#ifndef FACE_TOOLS_METRIC_LINE_ANGLE_METRIC_CALCULATOR_TYPE_H
#define FACE_TOOLS_METRIC_LINE_ANGLE_METRIC_CALCULATOR_TYPE_H

#include <MetricCalculatorTypeInterface.h>
#include <AngleVisualiser.h>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT LineAngleMetricCalculatorType : public MetricCalculatorTypeInterface
{ Q_OBJECT
public:
    LineAngleMetricCalculatorType( int id=-1, const Landmark::LmkList* l0=nullptr, const Landmark::LmkList* l1=nullptr);

    MCT* make( int id, const Landmark::LmkList*, const Landmark::LmkList*) const override;

    QString category() const override { return "Line Angle";}
    Vis::MetricVisualiser* visualiser() override { return &_vis;}
    bool canCalculate( const FM*, int, const Landmark::LmkList*) const override;
    void measure( std::vector<double>&, const FM*, int, const Landmark::LmkList*) const override;

private:
    Vis::AngleVisualiser _vis;
};  // end class

}}   // end namespaces

#endif
