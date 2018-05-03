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

#include <BaseVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <vtkProperty.h>
#include <vtkMapper.h>
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::SurfaceVisualisation;
using FaceTools::Vis::PointsVisualisation;
using FaceTools::Vis::WireframeVisualisation;
using FaceTools::Vis::TextureVisualisation;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


BaseVisualisation::BaseVisualisation( const QString& dname, const QIcon& icon, const QKeySequence& keys)
    : _dname(dname), _icon(NULL), _keys(NULL)
{
    _icon = new QIcon(icon);
    _keys = new QKeySequence(keys);
}   // end ctor


BaseVisualisation::BaseVisualisation( const QString& dname, const QIcon& icon)
    : _dname(dname), _icon(NULL), _keys(NULL)
{
    _icon = new QIcon(icon);
}   // end ctor


BaseVisualisation::BaseVisualisation( const QString& dname)
    : _dname(dname), _icon(NULL), _keys(NULL)
{
}   // end ctor


BaseVisualisation::~BaseVisualisation()
{
    if ( _icon)
        delete _icon;
    if ( _keys)
        delete _keys;
}   // end dtor


void SurfaceVisualisation::apply( const FaceControl* fc)
{
    vtkActor* actor = fc->view()->surfaceActor();
    actor->GetMapper()->SetScalarVisibility(false);
    actor->GetProperty()->SetRepresentationToSurface();
}   // end apply


void PointsVisualisation::apply( const FaceControl* fc)
{
    SurfaceVisualisation::apply(fc);
    fc->view()->surfaceActor()->GetProperty()->SetRepresentationToPoints();
}   // end apply


void WireframeVisualisation::apply( const FaceControl* fc)
{
    SurfaceVisualisation::apply(fc);
    fc->view()->surfaceActor()->GetProperty()->SetRepresentationToWireframe();
}   // end apply


void SurfaceVisualisation::addActors( const FaceControl* fc) { fc->viewer()->add(fc->view()->surfaceActor());}
void SurfaceVisualisation::removeActors( const FaceControl* fc) { fc->viewer()->remove(fc->view()->surfaceActor());}
void SurfaceVisualisation::onSelected( const FaceControl* fc) { fc->viewer()->enableFloodLights(false);}
bool SurfaceVisualisation::belongs( const vtkProp *p, const FaceControl* fc) const { return fc->view()->surfaceActor() == p;}

void TextureVisualisation::addActors( const FaceControl* fc) { fc->viewer()->add(fc->view()->textureActor());}
void TextureVisualisation::removeActors( const FaceControl* fc) { fc->viewer()->remove(fc->view()->textureActor());}
void TextureVisualisation::onSelected( const FaceControl* fc) { fc->viewer()->enableFloodLights(true);}
bool TextureVisualisation::belongs( const vtkProp *p, const FaceControl* fc) const { return fc->view()->textureActor() == p;}

bool TextureVisualisation::isAvailable( const FaceModel* fm) const { return fm->model()->getNumMaterials() == 1;}
