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

#ifndef FACE_TOOLS_VIS_METRIC_VISUALISER_H
#define FACE_TOOLS_VIS_METRIC_VISUALISER_H

#include "BaseVisualisation.h"
#include <vtkTextActor.h>

namespace FaceTools {
    
namespace Metric {
class MetricType;
}   // end namespace

namespace Vis {

class FaceTools_EXPORT MetricVisualiser : public BaseVisualisation
{
public:
    MetricVisualiser();
    ~MetricVisualiser() override;
    void setMetric( const Metric::MetricType*);
    const Metric::MetricType *metric() const { return _metric;}

    const char* name() const override { return "MetricVisualiser (virtual)";}

    //bool belongs( const vtkProp*, const FV*) const;
    //bool isVisible( const FV*) const override;
    //void syncWithViewTransform( const FV*) override;

    bool applyToAllInViewer() const { return true;}
    bool applyToAllViewers() const { return true;}

    bool isAvailable( const FV*, const QPoint*) const override;

    float minAllowedOpacity() const override { return 0.10f;}
    float maxAllowedOpacity() const override { return 0.99f;}

    void apply( const FV*, const QPoint*) override;
    void purge( const FV*) override;

    void refresh( const FV *fv) override;
    void setVisible( FV *fv, bool v) override;

    virtual void setHighlighted( const FV*, bool) = 0;

    const std::unordered_set<const FV*>& applied() const { return _fvs;}

protected:
    virtual void doApply( const FV*) = 0;
    virtual void doPurge( const FV*) = 0;
    virtual void doRefresh( const FV*) = 0;
    virtual void doSetVisible( const FV*, bool) = 0;

private:
    const Metric::MetricType *_metric;
    std::unordered_set<const FV*> _fvs;
};  // end class

}}   // end namespaces

#endif
