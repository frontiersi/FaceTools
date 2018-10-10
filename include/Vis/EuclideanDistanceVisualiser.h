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

#ifndef FACE_TOOLS_VIS_EUCLIDEAN_DISTANCE_VISUALISER_H
#define FACE_TOOLS_VIS_EUCLIDEAN_DISTANCE_VISUALISER_H

#include <MetricVisualiser.h>
#include <vtkActor.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT EuclideanDistanceVisualiser : public MetricVisualiser
{ Q_OBJECT
public:
    // If bilat is true, this visualisation will create two separate line actors
    // to reflect the fact that at least one of {lmkA,lmkB} is a bilateral landmark.
    EuclideanDistanceVisualiser( int id, int lmkA, int lmkB, bool bilat=false);

    bool belongs( const vtkProp*, const FV*) const override;
    void pokeTransform( const FV*, const vtkMatrix4x4*) override;
    void fixTransform( const FV*) override;
    void apply( FV*, const QPoint* mc=nullptr) override;
    void clear( FV*) override;
    void purge( const FM*) override;

private:
    const int _lmkA;
    const int _lmkB;
    const bool _bilat;
    std::unordered_map<const FV*, vtkActor*> _actor0;
    std::unordered_map<const FV*, vtkActor*> _actor1;

    void apply( FV*, FaceLateral, FaceLateral, std::unordered_map<const FV*, vtkActor*>&);
    void clear( FV*, std::unordered_map<const FV*, vtkActor*>&);
};  // end class

}}   // end namespaces

#endif
