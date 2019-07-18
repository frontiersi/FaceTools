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

#ifndef FACE_TOOLS_VIS_ANGLE_VISUALISER_H
#define FACE_TOOLS_VIS_ANGLE_VISUALISER_H

#include <MetricVisualiser.h>
#include <Landmark.h>
#include <vtkAngleRepresentation3D.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT AngleVisualiser : public MetricVisualiser
{ Q_OBJECT
public:
    AngleVisualiser( int id, const Landmark::LmkList* l0=nullptr, const Landmark::LmkList* l1=nullptr);

    bool belongs( const vtkProp*, const FV*) const override;
    void syncActorsToData( const FV*, const cv::Matx44d&) override;
    bool isAvailable( const FM*) const override;
    bool isVisible( const FV*) const override;

    void setHighlighted( const FV*, bool) override;

protected:
    void doApply( const FV*) override;
    void doPurge( const FV*) override;
    void doSetVisible( const FV*, bool) override;

private:
    const Landmark::LmkList* _lmks0;
    const Landmark::LmkList* _lmks1;
    std::unordered_map<const FV*, vtkAngleRepresentation3D*> _angle0, _angle1;

    void applyActor( const FV*, const Landmark::LmkList*, std::unordered_map<const FV*, vtkAngleRepresentation3D*>&);
    void purgeActor( const FV*, std::unordered_map<const FV*, vtkAngleRepresentation3D*>&);
};  // end class

}}   // end namespaces

#endif
