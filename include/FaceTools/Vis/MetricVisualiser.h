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

#ifndef FACE_TOOLS_VIS_METRIC_VISUALISER_H
#define FACE_TOOLS_VIS_METRIC_VISUALISER_H

#include "BaseVisualisation.h"
#include <vtkTextActor.h>

namespace FaceTools {
    
namespace Metric { class MetricType;}

namespace Vis {

class FaceTools_EXPORT MetricVisualiser : public BaseVisualisation
{
public:
    MetricVisualiser();
    void setMetric( const Metric::MetricType*);
    const Metric::MetricType *metric() const { return _metric;}

    //bool belongs( const vtkProp*, const FV*) const override;
    //bool isVisible( const FV*) const override;
    //void syncTransform( const FV*) override;
    //void setVisible( FV*, bool) override;
    //void purge( const FV*) override;
    //void refresh( FV*) override;

    bool applyToAllInViewer() const override { return true;}
    bool applyToAllViewers() const override { return true;}

    float minAllowedOpacity() const override { return 0.00f;}
    float maxAllowedOpacity() const override { return 0.99f;}

    bool isAvailable( const FV*) const override;

    virtual void setHighlighted( bool) = 0;

private:
    const Metric::MetricType *_metric;
};  // end class

}}   // end namespaces

#endif
