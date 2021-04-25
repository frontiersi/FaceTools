/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_METRIC_DEPTH_METRIC_TYPE_H
#define FACE_TOOLS_METRIC_DEPTH_METRIC_TYPE_H

#include "MetricType.h"
#include <FaceTools/Vis/DepthVisualiser.h>

namespace FaceTools { namespace Metric {

struct FaceTools_EXPORT DepthMeasure
{
    Vec3f p0; // Point on line segment being where the measurement was taken from
    Vec3f p1; // Point on the surface where the measurement was taken to
};  // end struct


class FaceTools_EXPORT DepthMetricType : public MetricType
{
public:
    DepthMetricType();

    QString category() const override { return "Depth";}
    QString units() const override { return FM::LENGTH_UNITS;}
    QString typeRemarks() const override { return "Depth is measured from a specific landmark (or weighted combination of landmarks) to either another standard point referenced by landmarks, or the facial surface itself. In-plane measurements are always taken at right angles to one of the standard facial planes.";}
    Vis::MetricVisualiser* visualiser() override { return &_vis;}

    bool fixedInPlane() const override { return false;}

    bool hasMeasurement( const FM *fm) const override { return _depthInfo.count(fm) > 0;}
    void purge( const FM *fm) override { _depthInfo.erase(fm);}

    const std::vector<DepthMeasure> &depthInfo( const FM *fm) const { return _depthInfo.at(fm);}

protected:
    float update( size_t, const FM*, const std::vector<Vec3f>&, Vec3f, Vec3f, bool, bool) override;

private:
    Vis::DepthVisualiser _vis;
    std::unordered_map<const FM*, std::vector<DepthMeasure> > _depthInfo;
};  // end class

}}   // end namespaces

#endif
