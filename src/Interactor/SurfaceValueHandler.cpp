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

#include <Interactor/SurfaceValueHandler.h>
#include <Vis/ColourVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceTools.h>
#include <ModelSelect.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
using FaceTools::Interactor::SurfaceValueHandler;
using FaceTools::Action::FaceAction;
using MS = FaceTools::ModelSelect;
using FV = FaceTools::Vis::FaceView;


SurfaceValueHandler::Ptr SurfaceValueHandler::create() { return Ptr( new SurfaceValueHandler);}


SurfaceValueHandler::SurfaceValueHandler() : _vwr(nullptr)
{
    _caption->BorderOff();
    _caption->LeaderOff();
    _caption->GetCaptionTextProperty()->BoldOff();
    _caption->GetCaptionTextProperty()->ItalicOff();
    _caption->GetCaptionTextProperty()->ShadowOn();
    _caption->GetCaptionTextProperty()->SetFontFamilyToCourier();
    _caption->GetCaptionTextProperty()->SetFontSize(15);
    _caption->SetPickable(false);
    _caption->GetTextActor()->SetTextScaleModeToNone();
}   // end ctor


void SurfaceValueHandler::_setCaptionColour( const QColor &fg)
{
    _caption->GetCaptionTextProperty()->SetColor( fg.redF(), fg.greenF(), fg.blueF());
}   // end _setCaptionColour


void SurfaceValueHandler::refresh()
{
    if (_vwr)
    {
        _vwr->remove(_caption);
        _vwr = nullptr;
    }   // end if

    FV *fv = MS::selectedView();
    if ( fv)
    {
        FMV *fmv = fv->viewer();
        assert(fmv);
        const QColor fg = chooseContrasting( fmv->backgroundColour());
        _setCaptionColour( fg);
        fmv->add(_caption);
        _vwr = fmv;
    }   // end if

    _caption->SetVisibility(false);
    setEnabled( fv && fv->activeColours());
}   // end refresh


bool SurfaceValueHandler::doMouseMove()
{
    const FV *fv = MS::selectedView();
    if ( !fv || !fv->activeColours())
    {
        setEnabled(false);
        return false;
    }   // end if

    const QPoint mc = fv->viewer()->mouseCoords();
    Vec3f v;
    bool onSurface = fv->projectToSurface( mc, v);
    int capSize = 0;
    if ( onSurface)
    {
        const QString cap = fv->activeColours()->getCaption(v);
        _caption->SetCaption( cap.toStdString().c_str());
        capSize = cap.size();
    }   // end if

    if ( capSize > 0)
    {
        double ap[3] = {double(v[0]), double(v[1]), double(v[2])};
        _caption->SetAttachmentPoint( ap);
        _caption->SetPosition( -capSize*4,-40);
    }   // end if

    _caption->SetVisibility( capSize > 0);
    return capSize > 0;
}   // end doMouseMove
