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

#ifndef FACE_TOOLS_FACE_CONTROL_SET_H
#define FACE_TOOLS_FACE_CONTROL_SET_H

#include "FaceControl.h"
#include "Hashing.h"
#include <vtkProp.h>
#include <unordered_set>
#include <unordered_map>

namespace FaceTools {

class FaceModelViewer;
class FaceModel;
typedef std::unordered_set<FaceModel*>       FaceModelSet;
typedef std::unordered_set<FaceModelViewer*> FaceViewerSet;

class FaceTools_EXPORT FaceControlSet
{
public:
    FaceControlSet(){}
    FaceControlSet( const FaceControlSet&);
    FaceControlSet& operator=( const FaceControlSet&);

    FaceControlSet& operator+( const FaceControlSet&);  // Add the given set to this set (union)
    FaceControlSet& operator-( const FaceControlSet&);  // Remove the given set from this set (difference)
    FaceControlSet& operator/( const FaceControlSet&);  // Remove from this set those NOT in given set (intersection)
    FaceControlSet operator()( const FaceModel*) const; // Return the set of FaceControls matching the given model.

    bool insert( FaceControl*);             // Returns true on successful insert.
    bool erase( FaceControl*);              // Returns true on successful erasure.
    bool erase( const FaceModel*);          // Erase all FaceControl instances that have the given FaceModel as their data.
    void clear();                           // Clear contents.

    bool has( FaceControl*) const;          // Returns true iff FaceControl present.
    bool has( const FaceModel*) const;      // Returns true iff at least one FaceControl with the given data is present.
    size_t size() const;                    // How many FaceControl instances.
    size_t size( const FaceModel*) const;   // Returns the number of FaceControl instances that map to the given FaceModel.
    bool empty() const;                     // True iff empty.
    FaceControl* first() const;             // Returns *_fcs.begin() or null if empty set.

    FaceViewerSet viewers() const;          // Return the set of viewers used at this moment by ALL REFERENCED FACEMODELS
    FaceViewerSet directViewers() const;    // Return the viewers just associated with the FaceControls in this set.
    const FaceModelSet& models() const;     // Return the set of models (FaceControl::data) from the set.

    // Given a prop, returns the associated FaceControl where FaceControl::belongs returns
    // true or null if not present. Currently does a linear lookup. TODO make hashable.
    FaceControl* find( const vtkProp*) const;

    std::unordered_set<FaceControl*>::iterator begin() { return _fcs.begin();}
    std::unordered_set<FaceControl*>::const_iterator begin() const { return _fcs.begin();}
    std::unordered_set<FaceControl*>::iterator end() { return _fcs.end();}
    std::unordered_set<FaceControl*>::const_iterator end() const { return _fcs.end();}

private:
    std::unordered_set<FaceControl*> _fcs;
    std::unordered_map<const FaceModel*, std::unordered_set<FaceControl*> > _fmm;
    FaceModelSet _fms;
};  // end class

}   // end namespace

Q_DECLARE_METATYPE( FaceTools::FaceControlSet)
    
#endif
