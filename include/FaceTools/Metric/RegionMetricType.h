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

#ifndef FACE_TOOLS_METRIC_REGION_METRIC_TYPE_H
#define FACE_TOOLS_METRIC_REGION_METRIC_TYPE_H

#include "MetricType.h"
#include <FaceTools/Vis/RegionVisualiser.h>

namespace FaceTools { namespace Metric {

struct FaceTools_EXPORT RegionMeasure
{
    std::vector<Vec3f> points;
};  // end struct


class FaceTools_EXPORT RegionMetricType : public MetricType
{
public:
    RegionMetricType();

    QString category() const override { return "Region";}
    QString units() const override { return "perimeter^2/area";}
    QString typeRemarks() const override { return "The perimeter of a region is the sum of the lengths of the individual line segments between an ordered list of landmarks. The area is defined according to the sum of the areas of the triangles composed by the landmarks.";}
    Vis::MetricVisualiser* visualiser() override { return &_vis;}

    bool fixedInPlane() const override { return false;}
    void setInPlane( bool v) override { _inPlane = v;}
    bool inPlane() const override { return _inPlane;}

    void purge( const FM *fm) override { _regionInfo.erase(fm);}
    const std::vector<RegionMeasure> &regionInfo( const FM *fm) const { return _regionInfo.at(fm);}

protected:
    float update( size_t, const FM*, const std::vector<Vec3f>&, Vec3f, Vec3f, bool, bool) override;

private:
    Vis::RegionVisualiser _vis;
    bool _inPlane;
    std::unordered_map<const FM*, std::vector<RegionMeasure> > _regionInfo;
};  // end class

}}   // end namespaces

#endif
