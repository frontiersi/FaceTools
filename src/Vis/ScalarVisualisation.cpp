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

#include <ScalarVisualisation.h>
#include <LegendScalarColourRangeMapper.h>
#include <FaceControl.h>
#include <FaceModelViewer.h>
#include <FaceView.h>
#include <vtkProperty.h>
#include <vtkMapper.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::SurfaceVisualisation;
using FaceTools::Vis::ScalarVisualisation;
using FaceTools::FaceControl;
using FaceTools::ModelViewer;
using QTools::ColourMappingWidget;
typedef FaceTools::Vis::LegendScalarColourRangeMapper LSCRM;

ScalarVisualisation::ScalarVisualisation( const QString& d, const QIcon& i, const QKeySequence& k)
    : SurfaceVisualisation(d,i,k) {}   // end ctor

ScalarVisualisation::ScalarVisualisation( const QString& d, const QIcon& i)
    : SurfaceVisualisation(d,i) {}   // end ctor

ScalarVisualisation::ScalarVisualisation( const QString& d)
    : SurfaceVisualisation(d) {}   // end ctor


ScalarVisualisation::~ScalarVisualisation()
{
    std::for_each( std::begin(_lranges), std::end(_lranges), [](auto f){ delete f.second;});
    _lranges.clear();
}   // end dtor


void ScalarVisualisation::apply( const FaceControl* fc)
{
    vtkActor* actor = fc->view()->surfaceActor();
    if ( _lranges.count(fc) == 0)
    {
        LSCRM* leg = _lranges[fc] = new LSCRM( actor, false/*don't auto-remap*/);
        leg->setRangeLimits( getDisplayName().toStdString(), rangeMin(), rangeMax()); // set defaults
    }   // end if
    mapSurfaceActor( fc);   // Apply visualisation to the LSCRM
    actor->GetMapper()->SetScalarVisibility(true);
}   // end apply


void ScalarVisualisation::addActors( const FaceControl* fc)
{
    SurfaceVisualisation::addActors(fc);
    onSelected(fc);
}   // end addActors


void ScalarVisualisation::removeActors( const FaceControl* fc)
{
    SurfaceVisualisation::removeActors(fc);
    _lranges.at(fc)->setVisible("", NULL);  // NULL viewer hides
}   // end removeActors


void ScalarVisualisation::onSelected( const FaceControl* fc)
{
    assert(fc);
    assert(_lranges.count(fc) > 0);
    // Ensure the legend mapping is selected for display in the corresponding viewer's scalar legend.
    std::string nm = getDisplayName().toStdString();
    _lranges.at(fc)->setVisible( nm, fc->viewer());
}   // end onSelected


void ScalarVisualisation::burn( const FaceControl* fc)
{
    if (_lranges.count(fc) > 0)
    {
        delete _lranges.at(fc);
        _lranges.erase(fc);
    }   // end if
}   // end burn


bool ScalarVisualisation::updateColourMapping( const FaceControl* fc, const ColourMappingWidget* w)
{
    assert(w);
    assert(fc);
    if ( _lranges.count(fc) == 0)
        return false;

    LSCRM* leg = _lranges.at(fc);
    QColor mcol0 = w->minColour();
    QColor mcol1 = w->midColour();
    QColor mcol2 = w->maxColour();
    leg->setColours( mcol0, mcol1, mcol2);
    leg->setNumColours( w->numColours());
    leg->setVisibleRange( w->minScalar(), w->maxScalar());
    leg->remap();
    fc->viewer()->updateRender();
    return true;
}   // end doOnWidgetChanged


bool ScalarVisualisation::updateWidget( ColourMappingWidget* w, const FaceControl* fc) const
{
    assert(w);
    assert(fc);
    if (_lranges.count(fc) == 0)
        return false;

    LSCRM* leg = _lranges.at(fc);
    QColor c0, c1, c2;
    leg->colours(c0,c1,c2);
    const auto& vrng = *leg->visibleRange();  // Currently set values.
    const float rmin = rangeMin();  // Could get from LSCRM but is same
    const float rmax = rangeMax();  // Could get from LSCRM but is same
    const bool threeband = rmin < 0.0f && rmax > 0.0f;  // Will tell widget whether or not to hide the "mid" colour

    w->setThreeBand( threeband);
    w->setMinColour(c0);
    w->setMidColour(c1);
    w->setMaxColour(c2);
    w->setScalarRangeLimits( rmin, rmax);
    w->setMinScalar( vrng.first);
    w->setMaxScalar( vrng.second);
    w->setNumColours( leg->numColours());
    return true;
}   // end setWidgetTo
