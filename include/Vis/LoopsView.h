/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#include <ModelViewer.h>
#include <Hashing.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT LoopsView
{
public:
    LoopsView( float lineWidth=1.0f, float red=1.0f, float green=1.0f, float blue=1.0f);
    virtual ~LoopsView();

    void deleteActors();

    // Add a new loop actor - does NOT set visible in viewer.
    void addLoop( const std::list<cv::Vec3f>&);

    // Add as a bunch of vertices.
    void addPoints( const std::vector<cv::Vec3f>&);

    // Set/get line width and colour.
    void setLineWidth( float lw);
    float lineWidth() const { return _lineWidth;}

    void setColour( float red, float green, float blue);
    void setColour( const cv::Vec3f& c);
    const cv::Vec3f& colour() const { return _colour;}

    void setVisible( bool, ModelViewer*);
    bool visible() const { return _visible;}

    void pokeTransform( const vtkMatrix4x4*);
    void fixTransform();

private:
    bool _visible;
    float _lineWidth;
    cv::Vec3f _colour;
    std::unordered_set<vtkActor*> _actors;

    LoopsView( const LoopsView&) = delete;
    void operator=( const LoopsView&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
