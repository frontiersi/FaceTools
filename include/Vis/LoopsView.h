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

#ifndef FACE_TOOLS_LOOPS_VIEW_H
#define FACE_TOOLS_LOOPS_VIEW_H

#include <FaceTypes.h>
#include <ModelViewer.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT LoopsView
{
public:
    LoopsView( float lineWidth=1.0f, double red=1.0, double green=1.0, double blue=1.0);
    LoopsView( const LoopsView&);
    LoopsView& operator=( const LoopsView&);
    virtual ~LoopsView();

    void deleteActors();

    // Add a new loop actor - does NOT set visible in viewer.
    void addLoop( const std::list<cv::Vec3f>&);

    // Add as a bunch of vertices.
    void addPoints( const std::vector<cv::Vec3f>&);

    // Set/get line width to be applied to newly added loops/points.
    void setLineWidth( float lw);
    float lineWidth() const { return _lineWidth;}

    // Set/get colour to be applied to newly added loops/points.
    void setColour( double red, double green, double blue);

    // Change the colour of existing actors.
    void changeColour( double red, double green, double blue);

    void setVisible( bool, ModelViewer*);
    bool visible() const { return _visible;}

    void pokeTransform( const vtkMatrix4x4*);

private:
    bool _visible;
    float _lineWidth;
    cv::Vec3d _colour;
    std::vector<vtkActor*> _actors;
    std::vector< std::list<cv::Vec3f> > _lines;
    std::vector< std::vector<cv::Vec3f> > _points;
};  // end class

}}   // end namespaces

#endif
