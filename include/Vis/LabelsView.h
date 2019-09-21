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

#ifndef FACE_TOOLS_VIS_LABELS_VIEW_H
#define FACE_TOOLS_VIS_LABELS_VIEW_H

#include <FaceTypes.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkPolyData.h>
#include <vtkActor2D.h>
#include <vtkActor.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT LabelsView
{
public:
    LabelsView( int fontSize, bool boldOn);
    virtual ~LabelsView(){}

    void refresh( const RFeatures::ObjModel&);

    void setVisible( bool, ModelViewer*);
    bool visible() const { return _visible;}

    virtual void setColours( const QColor& fg, const QColor& bg);

    // Transforming the view also sets moving to false.
    void transform( const cv::Matx44d&);

    // Set moving to true only when the parent model's view transform doesn't agree with its model transform.
    void setMoving( bool v) { _moving = v;}

    bool moving() const { return _moving;}

protected:
    virtual vtkSmartPointer<vtkPolyData> createLabels( const RFeatures::ObjModel&) const = 0;

private:
    vtkSmartPointer<vtkActor> _actor;
    vtkSmartPointer<vtkActor2D> _labels;
    vtkSmartPointer<vtkPointSetToLabelHierarchy> _filter;
    bool _visible;
    bool _moving;
    cv::Matx44d _lt;    // Last transform applied
};  // end class

}}   // end namespace

#endif
