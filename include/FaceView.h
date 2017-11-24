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

#ifndef FACE_TOOLS_FACE_VIEW_H
#define FACE_TOOLS_FACE_VIEW_H

#include "ModelOptions.h"
#include <ObjModel.h>   // RFeatures
#include <vtkSmartPointer.h>
#include <vtkActor.h>

namespace FaceTools
{

class ModelViewer;

class FaceTools_EXPORT FaceView
{
public:
    explicit FaceView( const RFeatures::ObjModel::Ptr);
    virtual ~FaceView();

    void reset( const RFeatures::ObjModel::Ptr);

    void setVisible( bool, ModelViewer* viewer=NULL);
    ModelViewer* getViewer() { return _viewer;}
    bool isVisible() const;

    vtkActor* getActor() const;   // Texture or surface depending on isTexture()
    vtkActor* getSurfaceActor() const;

    void setTexture( bool); // Set model for next visualisation
    bool isTexture() const; // If true, getActor() returns getTextureActor()

    void setOptions( const ModelOptions&);
    const boost::unordered_map<int,int>* getPolyIdLookups() const;
 
private:
    ModelViewer *_viewer;
    bool _isshown;
    bool _istexture;
    vtkSmartPointer<vtkActor> _tactor;
    vtkSmartPointer<vtkActor> _sactor;
    ModelOptions _opts;
    boost::unordered_map<int,int> _flookup;

    FaceView( const FaceView&);       // No copy
    void operator=( const FaceView&); // No copy
};  // end class

}   // end namespace

#endif
