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

#ifndef FACE_TOOLS_VIS_COLOUR_VISUALISATION_H
#define FACE_TOOLS_VIS_COLOUR_VISUALISATION_H

#include "BaseVisualisation.h"
#include <QTools/ScalarColourRangeMapper.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT ColourVisualisation : public BaseVisualisation
{
public:
    /**
     * label:  Readable name of the visualisation
     * minv:   minimum allowed metric value
     * maxv:   maximum allowed metric value
     * step:   value step size - if default, computed as 20 divisions in maxv - minv
     * nums:   number of colour bands to step through at once.
     */
    ColourVisualisation( const QString &label, float minv, float maxv, float step=0.0f, size_t nums=1);

    const QString &label() const { return _label;}

    float minRange() const { return _minr;} // Min range initially same as min visible
    float maxRange() const { return _maxr;} // Max range initially same as max visible
    float minVisible() const { return _cmapper.minVisible();}
    float maxVisible() const { return _cmapper.maxVisible();}
    float stepSize() const { return _ssize;}
    void setNumStepSize( size_t v) { _nssize = v;}
    size_t numStepSize() const { return _nssize;}

    // Return the number of colours used to map the surface of actors (never less than 1).
    size_t numColours() const { return _cmapper.numColours();}
    void setNumColours( size_t v) { _cmapper.setNumColours(v);}
    QColor minColour() const { return _cmapper.minColour();}
    QColor maxColour() const { return _cmapper.maxColour();}
    void setMinColour( const QColor& c) { _cmapper.setMinColour(c);}
    void setMaxColour( const QColor& c) { _cmapper.setMaxColour(c);}

    // Return a caption for the value corresponding to the given point on the
    // model for this visualisation. Will be displayed next to the mouse cursor.
    virtual QString getCaption( const Vec3f&) const { return "";}

    void setVisibleRange( float, float);    // Scalar visible values
    void rebuildColourMapping();      // Call after changing colour mapping aspects

    const vtkLookupTable* lookupTable() const { return _lut.Get();}

    bool isVisible( const FV* fv) const override { return fv->activeColours() == this;}
    void setVisible( FV* fv, bool v) override;
    void activate( FV*);    // Called by FaceView
    void deactivate( FV*);  // Called by FaceView

    //bool isAvailable( const FV *fv) const override;
    //void refresh( FV* fv) override
    //void syncTransform( const FV*) override;
    //void purge( const FV*) override;

    static void addCellsArray( vtkActor*, vtkFloatArray*);
    static void addPointsArray( vtkActor*, vtkFloatArray*);
    static void setActiveCellScalars( vtkActor*, const char *name="");
    static void setActiveCellVectors( vtkActor*, const char *name="");
    static void setActivePointScalars( vtkActor*, const char *name="");
    static void setActivePointVectors( vtkActor*, const char *name="");

protected:
    // Only need to implement this if other actors in the subclassed
    // visualisation depend upon updates to the lookup table. Called for
    // every FaceView since the actors are probably associated with them.
    // Only ever called on currently visible visualisations.
    virtual void show( FV*){}
    virtual void hide( FV*){}
    virtual void refreshColourMap( const FV*){}

private:
    const QString _label;
    const float _minr;
    const float _maxr;
    float _ssize;
    size_t _nssize;
    QTools::ScalarColourRangeMapper _cmapper;
    std::unordered_set<FV*> _visible;
    vtkSmartPointer<vtkLookupTable> _lut;
    void _refreshLookupTable( FV*);
};  // end class

}}   // end namespace

#endif
