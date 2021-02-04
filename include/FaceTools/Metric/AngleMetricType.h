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

#ifndef FACE_TOOLS_METRIC_ANGLE_METRIC_TYPE_H
#define FACE_TOOLS_METRIC_ANGLE_METRIC_TYPE_H

#include "MetricType.h"
#include <FaceTools/Vis/AngleVisualiser.h>

namespace FaceTools { namespace Metric {

struct FaceTools_EXPORT AngleMeasure
{
    Vec3f point0;
    Vec3f point1;
    Vec3f centre;
    Vec3f normal;
    float degrees;
};  // end struct


class FaceTools_EXPORT AngleMetricType : public MetricType
{
public:
    AngleMetricType();

    QString category() const override { return "Angle";}
    QString units() const override { return "degrees";}
    QString typeRemarks() const override
    { return "Angles are always measured within an anatomical plane - typically either the median or the coronal plane.";}

    Vis::MetricVisualiser* visualiser() override { return &_vis;}

    bool hasMeasurement( const FM *fm) const override { return _angleInfo.count(fm) > 0;}
    void purge( const FM *fm) override { _angleInfo.erase(fm);}
    const std::vector<AngleMeasure> &angleInfo( const FM *fm) const { return _angleInfo.at(fm);}

protected:
    float update( size_t, const FM*, const std::vector<Vec3f>&, Vec3f, Vec3f, bool, bool) override;

private:
    Vis::AngleVisualiser _vis;
    std::unordered_map<const FM*, std::vector<AngleMeasure> > _angleInfo;
};  // end class

}}   // end namespaces

#endif
