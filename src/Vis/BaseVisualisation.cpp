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

#include <Vis/BaseVisualisation.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <vtkProperty.h>
#include <vtkMapper.h>
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::WireframeVisualisation;
using FaceTools::Vis::TextureVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Action::Event;

bool BaseVisualisation::isAvailable( const FV* fv, const QPoint*) const { return isAvailable(fv->data());}

void TextureVisualisation::apply( FV* fv, const QPoint*) { setVisible(fv, true);}
bool TextureVisualisation::purge( FV* fv, Event) { setVisible(fv, false); return true;}
void TextureVisualisation::setVisible( FV* fv, bool v) { fv->setTextured( v && fv->canTexture());}
bool TextureVisualisation::isVisible( const FV* fv) const { return fv->textured() || (!fv->canTexture() && fv->activeSurface() == nullptr);}
bool TextureVisualisation::belongs( const vtkProp *p, const FV* fv) const { return fv->actor() == p;}

void WireframeVisualisation::apply( FV* fv, const QPoint*) { setVisible(fv, true);}
bool WireframeVisualisation::purge( FV* fv, Event) { setVisible(fv,false); return true;}
void WireframeVisualisation::setVisible( FV* fv, bool v) { fv->setWireframe( v);}
bool WireframeVisualisation::isVisible( const FV* fv) const { return fv->wireframe();}
bool WireframeVisualisation::belongs( const vtkProp *p, const FV* fv) const { return fv->actor() == p;}
