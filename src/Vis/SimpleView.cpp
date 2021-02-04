/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Vis/SimpleView.h>
#include <r3dvis/VtkTools.h>
#include <vtkImageData.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
using FaceTools::Vis::SimpleView;
using FaceTools::ModelViewer;
using FaceTools::Mat4f;


vtkSmartPointer<vtkTexture> SimpleView::s_stippleTexture;   // static


SimpleView::SimpleView() : _vwr(nullptr), _visible(false) {}


SimpleView::~SimpleView() { setVisible(false, _vwr);}


vtkActor* SimpleView::addActor( vtkPolyDataAlgorithm *src)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( src->GetOutputPort());
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper( mapper);
    actor->SetPickable( false);
    initActor(actor);
    return actor;
}   // end addActor


vtkProperty* SimpleView::initActor( vtkSmartPointer<vtkActor> actor)
{
    _actors.push_back(actor);
    vtkProperty* prop = actor->GetProperty();
    prop->SetAmbient( 1.0);
    prop->SetDiffuse( 0.0);
    prop->SetSpecular( 0.0);
    prop->SetOpacity( 0.99);
    setLineStipplingEnabled(true);  // Necessary
    setLineStipplingEnabled(false); // Yes, this too
    return prop;
}   // end initActor


void SimpleView::setColour( double r, double g, double b, double a)
{
    for ( vtkActor* actor : _actors)
        setActorColour( actor, r, g, b, a);
}   // end setColour


void SimpleView::setActorColour( vtkActor *actor, double r, double g, double b, double a)
{
    r = std::max( 0.0, std::min( r, 1.0));
    g = std::max( 0.0, std::min( g, 1.0));
    b = std::max( 0.0, std::min( b, 1.0));
    vtkProperty* prop = actor->GetProperty();
    prop->SetColor( r, g, b);
    if ( a >= 0.0 && a <= 1.0)
        prop->SetOpacity( a);
}   // end setActorColour


void SimpleView::setLineWidth( double lw)
{
    for ( vtkActor* actor : _actors)
    {
        vtkProperty* prop = actor->GetProperty();
        prop->SetLineWidth( lw);
    }   // end for
}   // end setLineWidth


void SimpleView::pokeTransform( const vtkMatrix4x4* d)
{
    for ( vtkActor* actor : _actors)
        pokeTransform( actor, d);
}   // end pokeTransform


void SimpleView::pokeTransform( vtkActor* actor, const Mat4f& m)
{
    vtkSmartPointer<vtkMatrix4x4> vm = r3dvis::toVTK(m);
    pokeTransform( actor, vm);
}   // end pokeTransform


void SimpleView::pokeTransform( vtkActor* actor, const vtkMatrix4x4* d)
{
    actor->PokeMatrix( const_cast<vtkMatrix4x4*>(d));
}   // end pokeTransform


void SimpleView::setVisible( bool visible, ModelViewer* vwr)
{
    if ( !visible)
    {
        for ( vtkActor* actor : _actors)
        {
            if ( _vwr)
                _vwr->remove(actor);
            if ( vwr)
                vwr->remove(actor);
        }   // end for
        _visible = false;
    }   // end if

    _vwr = vwr;
    
    if ( visible && _vwr)
    {
        for ( vtkActor* actor : _actors)
            _vwr->add(actor);
        _visible = true;
    }   // end if
}   // end setVisible


bool SimpleView::belongs( const vtkProp* prop) const
{
    for ( vtkActor* actor : _actors)
        if ( prop == actor)
            return true;
    return false;
}   // end belongs


void SimpleView::setLineStipplingEnabled( bool enabled)
{
    if ( enabled)
    {
        for ( vtkSmartPointer<vtkActor> actor : _actors)
        {
            _setStippleTextureCoords( actor);
            actor->SetTexture( _stippleTexture());
        }   // end for
    }   // end if
    else
    {
        for ( vtkSmartPointer<vtkActor> actor : _actors)
        {
            r3dvis::getPolyData( actor)->GetPointData()->SetTCoords(nullptr);
            actor->SetTexture( nullptr);
        }   // end for
    }   // end else
}   // end setLineStipplingEnabled


vtkSmartPointer<vtkTexture> SimpleView::_stippleTexture()
{
    if ( s_stippleTexture)
        return s_stippleTexture;

    static const int lineStipplePattern = 0xAAAAAAAA;
    static const int lineStippleRepeat = 7;

    // Create the image
    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    const int dimension = 16 * lineStippleRepeat;
    image->SetDimensions( dimension, 1, 1);
    image->AllocateScalars( VTK_UNSIGNED_CHAR, 4);
    image->SetExtent( 0, dimension - 1, 0, 0, 0, 0);
    unsigned char *pixel = static_cast<unsigned char *>(image->GetScalarPointer());
    const unsigned char ON = 255;
    const unsigned char OFF = 0;
    for ( int i = 0; i < 16; ++i)
    {
        const unsigned int mask = 1 << i;
        const unsigned int bit = (lineStipplePattern & mask) >> i;
        if ( static_cast<unsigned char>(bit) == 0)
        {
            for ( int j = 0; j < lineStippleRepeat; ++j)
            {
                *pixel       = ON;
                *(pixel + 1) = ON;
                *(pixel + 2) = ON;
                *(pixel + 3) = OFF;
                pixel += 4;
            }   // end for
        }   // end if
        else
        {
            for ( int j = 0; j < lineStippleRepeat; ++j)
            {
                *pixel       = ON;
                *(pixel + 1) = ON;
                *(pixel + 2) = ON;
                *(pixel + 3) = ON;
                pixel += 4;
            }   // end for
        }   // end else
    }   // end for

    // Create the texture from the image
    s_stippleTexture = vtkSmartPointer<vtkTexture>::New();
    s_stippleTexture->SetInputData(image);
    s_stippleTexture->InterpolateOff();
    s_stippleTexture->RepeatOn();
    return s_stippleTexture;
}   // end _stippleTexture


void SimpleView::_setStippleTextureCoords( vtkActor *actor)
{
    // Create texture coordinates for the given actor
    vtkPolyData *polyData = r3dvis::getPolyData( actor);
    vtkSmartPointer<vtkDoubleArray> tcoords = vtkSmartPointer<vtkDoubleArray>::New();
    tcoords->SetNumberOfComponents(1);
    tcoords->SetNumberOfTuples( polyData->GetNumberOfPoints());
    for ( int i = 0; i < polyData->GetNumberOfPoints(); ++i)
    {
        const double value = 0.5 * i;
        tcoords->SetTypedTuple( i, &value);
    }   // end for
    polyData->GetPointData()->SetTCoords(tcoords);
}   // end _setStippleTextureCoords
