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

#ifndef FACE_TOOLS_VIS_REGION_VISUALISER_H
#define FACE_TOOLS_VIS_REGION_VISUALISER_H

#include "MetricVisualiser.h"
#include <LndMrk/Landmark.h>
#include <vtkActor.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT RegionVisualiser : public MetricVisualiser
{
public:
    RegionVisualiser( int id, const Landmark::LmkList* lmks0=nullptr, const Landmark::LmkList* lmks1=nullptr);

    bool belongs( const vtkProp*, const FV*) const override;
    void syncToViewTransform( const FV*, const vtkMatrix4x4*) override;
    bool isAvailable( const FM*) const override;
    bool isVisible( const FV*) const override;
    void checkState( const FV*) override;
    void setHighlighted( const FV*, bool) override;

protected:
    void doApply( const FV*) override;
    void doPurge( const FV*) override;
    void doSetVisible( const FV*, bool) override;

private:
    const Landmark::LmkList* _lmks0;
    const Landmark::LmkList* _lmks1;
    std::unordered_map<const FV*, vtkActor*> _rep0, _rep1;

    void applyActor( const FV*, const Landmark::LmkList*, std::unordered_map<const FV*, vtkActor*>&);
    void purgeActor( const FV*, std::unordered_map<const FV*, vtkActor*>&);
};  // end class

}}   // end namespaces

#endif
