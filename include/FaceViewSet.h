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
    FaceViewSet operator()( const FM*) const;         // Return the set of FaceControls matching the given model.

    size_t insert( const FaceViewSet&);     // Returns the number from the given set inserted into this set.
    bool insert( Vis::FV*);                 // Returns true on successful insert.
    bool erase( const Vis::FV*);            // Returns true on successful erasure.
    bool insert( const FM*);                // Insert all FaceControl instances belonging to the given FaceModel.
    bool erase( const FM*);                 // Erase all FaceControl instances belonging to the given FaceModel.
    void clear();                           // Clear contents.

    bool has( const Vis::FV*) const;        // Returns true if the given FaceControl is present.
    bool has( const FM*) const;             // Returns true iff at least one FaceControl with the given data is present.
    size_t size() const;                    // How many FaceControl instances.
    size_t size( const FM*) const;          // Returns the number of FaceControl instances that map to the given FaceModel.
    bool empty() const;                     // True iff empty.
    Vis::FV* first() const;                 // Returns *_fvs.begin() or null if empty set.

    FMVS viewers() const;          // Return the set of viewers used at this moment by ALL REFERENCED FACEMODELS
    FMVS directViewers() const;    // Return the viewers just associated with the FaceControls in this set.
    const FMS& models() const;     // Return the set of models (FaceControl::data) from the set.

    // Returns the FaceControl that the prop belongs to or null if prop not associated with any FV in this set.
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
