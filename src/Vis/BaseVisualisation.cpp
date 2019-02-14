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

#include <BaseVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <vtkProperty.h>
#include <vtkMapper.h>
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::WireframeVisualisation;
using FaceTools::Vis::TextureVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;


BaseVisualisation::BaseVisualisation( const QString& dname, const QIcon& icon, const QKeySequence& keys)
    : _dname(dname), _icon(nullptr), _keys(nullptr)
{
    _icon = new QIcon(icon);
    _keys = new QKeySequence(keys);
}   // end ctor


BaseVisualisation::BaseVisualisation( const QString& dname, const QIcon& icon)
    : _dname(dname), _icon(nullptr), _keys(nullptr)
{
    _icon = new QIcon(icon);
}   // end ctor


BaseVisualisation::BaseVisualisation( const QString& dname)
    : _dname(dname), _icon(nullptr), _keys(nullptr)
{
}   // end ctor


BaseVisualisation::~BaseVisualisation()
{
    if ( _icon)
        delete _icon;
    if ( _keys)
        delete _keys;
}   // end dtor


bool BaseVisualisation::isAvailable( const FV* fv, const QPoint*) const { return isAvailable(fv->data());}

void TextureVisualisation::apply( FV* fv, const QPoint*) { fv->setTextured(true);}
void TextureVisualisation::clear( FV* fv) { fv->setTextured(false); }
bool TextureVisualisation::belongs( const vtkProp *p, const FV* fv) const { return fv->actor() == p;}
bool TextureVisualisation::isAvailable( const FM* fm) const { return fm->fvs().first()->canTexture();}

void WireframeVisualisation::apply( FV* fv, const QPoint*) { fv->setWireframe(true);}
void WireframeVisualisation::clear( FV* fv) { fv->setWireframe(false);}
bool WireframeVisualisation::belongs( const vtkProp *p, const FV* fv) const { return fv->actor() == p;}
