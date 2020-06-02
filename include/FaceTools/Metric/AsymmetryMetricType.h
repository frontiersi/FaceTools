/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_METRIC_ASYMMETRY_METRIC_TYPE_H
#define FACE_TOOLS_METRIC_ASYMMETRY_METRIC_TYPE_H

#include "MetricType.h"
#include <FaceTools/Vis/AsymmetryVisualiser.h>

namespace FaceTools { namespace Metric {

struct FaceTools_EXPORT AsymmetryMeasure
{
    Vec3f point0;
    Vec3f point1;
    Vec3f delta;
};  // end struct


class FaceTools_EXPORT AsymmetryMetricType : public MetricType
{
public:
    AsymmetryMetricType();

    QString category() const override { return "Asymmetry";}
    QString units() const override { return FM::LENGTH_UNITS;}
    QString typeRemarks() const override;

    Vis::MetricVisualiser* visualiser() override { return &_vis;}

    void purge( const FM *fm) override { _asymmInfo.erase(fm);}
    const std::vector<AsymmetryMeasure> &asymmetryInfo( const FM *fm) const { return _asymmInfo.at(fm);}

protected:
    float update( size_t, const FM*, const std::vector<Vec3f>&, Vec3f, Vec3f, bool, bool) override;

private:
    Vis::AsymmetryVisualiser _vis;
    std::unordered_map<const FM*, std::vector<AsymmetryMeasure> > _asymmInfo;
};  // end class

}}   // end namespaces

#endif
