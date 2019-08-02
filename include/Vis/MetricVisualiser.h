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

#ifndef FACE_TOOLS_VIS_METRIC_VISUALISER_H
#define FACE_TOOLS_VIS_METRIC_VISUALISER_H

#include <BaseVisualisation.h>
#include <FaceViewSet.h>
#include <vtkTextActor.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT MetricVisualiser : public BaseVisualisation
{
public:
    explicit MetricVisualiser( int);    // Supply the associated metric ID
    ~MetricVisualiser() override;

    int id() const { return _id;}

    /*
    bool belongs( const vtkProp*, const FV*) const;
    void syncActorsToData( const FV*, const cv::Matx44d&) override
    bool isAvailable( const FM*) const override;
    bool isVisible( const FV*) const override;
    */

    void apply( FV*, const QPoint*) override;
    bool purge( FV*, Action::Event) override;

    void setVisible( FV* fv, bool v) override { doSetVisible(fv,v);}

    virtual void setHighlighted( const FV*, bool) = 0;

    const FVS& applied() const { return _fvs;}

protected:
    virtual void doApply( const FV*) = 0;
    virtual void doPurge( const FV*) = 0;
    virtual void doSetVisible( const FV*, bool) = 0;

private:
    int _id;
    FVS _fvs;
};  // end class

}}   // end namespaces

#endif
