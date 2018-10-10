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

#ifndef FACE_TOOLS_FACE_VIEW_SET_H
#define FACE_TOOLS_FACE_VIEW_SET_H

#include <FaceView.h>

namespace FaceTools {

class FaceTools_EXPORT FaceViewSet
{
public:
    typedef std::shared_ptr<FaceViewSet> Ptr;
    static Ptr create();
    static Ptr create( const FaceViewSet&);

    FaceViewSet(){}
    FaceViewSet( const FaceViewSet&);
    FaceViewSet& operator=( const FaceViewSet&);

    FaceViewSet& operator+( const FaceViewSet&);      // Add the given set to this set (union)
    FaceViewSet& operator-( const FaceViewSet&);      // Remove the given set from this set (difference)
    FaceViewSet& operator/( const FaceViewSet&);      // Remove from this set those NOT in given set (intersection)
    FaceViewSet operator()( const FM*) const;         // Return the set of FaceViews matching the given model.

    size_t insert( const FaceViewSet&); // Returns the number from the given set inserted into this set.
    bool insert( Vis::FV*);             // Returns true on successful insert.
    bool erase( const Vis::FV*);        // Returns true on successful erasure.
    bool insert( const FM*);            // Insert all FaceView instances belonging to the given FaceModel.
    bool erase( const FM*);             // Erase all FaceView instances belonging to the given FaceModel.
    void clear();                       // Clear contents.
    size_t includeModelViews();         // Include all FaceViews of all referenced FaceModels; returns new size.
    size_t includeViewerViews();        // Include all FaceViews of all referenced FaceModelViewers; returns new size.

    bool has( const Vis::FV*) const;    // Returns true if the given FaceView is present.
    bool has( const FM*) const;         // Returns true iff at least one FaceView with the given data is present.
    size_t size() const;                // How many FaceView instances.
    size_t size( const FM*) const;      // Returns the number of FaceView instances that map to the given FaceModel.
    bool empty() const;                 // True iff empty.
    Vis::FV* first() const;             // Returns *_fvs.begin() or null if empty set.

    const FMS& models() const;          // Return the set of models from this set of FaceViews.
    FMVS dviewers() const;              // Return the viewers that this set's FaceViews are currently attached to.
    void updateRenderers() const;       // Shortcut to call updateRender on the set of FMVs returned by dviewers.

    // Returns the FaceView that the prop belongs to or null if prop not associated with any FV in this set.
    // Currently does a linear lookup. TODO make hashable.
    Vis::FV* find( const vtkProp*) const;

    std::unordered_set<Vis::FV*>::iterator begin() { return _fvs.begin();}
    std::unordered_set<Vis::FV*>::const_iterator begin() const { return _fvs.begin();}
    std::unordered_set<Vis::FV*>::iterator end() { return _fvs.end();}
    std::unordered_set<Vis::FV*>::const_iterator end() const { return _fvs.end();}

private:
    std::unordered_set<Vis::FV*> _fvs;
    std::unordered_map<const FM*, std::unordered_set<Vis::FV*> > _fmm;
    FMS _fms;
};  // end class

}   // end namespace

Q_DECLARE_METATYPE( FaceTools::FaceViewSet)

#endif
