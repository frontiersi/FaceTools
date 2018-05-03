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

#include "FaceTools_Export.h"
#include "Hashing.h"
#include <vtkProp.h>
#include <unordered_set>

namespace FaceTools {

class FaceModelViewer;
class FaceControl;
class FaceModel;

typedef std::unordered_set<FaceModel*> FaceModelSet;
typedef std::unordered_set<FaceModelViewer*> FaceViewerSet;

class FaceTools_EXPORT FaceControlSet
{
public:
    FaceControlSet(){}
    FaceControlSet( const FaceControlSet&);
    FaceControlSet& operator=( const FaceControlSet&);

    FaceControlSet& operator+( const FaceControlSet&); // Add the given set to this set (union)
    FaceControlSet& operator-( const FaceControlSet&); // Remove the given set from this set (difference)
    FaceControlSet& operator/( const FaceControlSet&); // Remove from this set those NOT in given set (intersection)

    bool insert( FaceControl*);         // Returns true on successful insert
    bool erase( FaceControl*);          // Returns true on successful erasure
    bool has( FaceControl*) const;      // Returns true iff present
    size_t count( FaceControl*) const;  // Returns 1 iff present, else 0.
    size_t size() const;                // How many members.
    bool empty() const;                 // True iff empty.
    void clear();                       // Clear contents.
    FaceControl* first() const;         // Returns *_fcs.begin() or NULL if empty set.

    std::unordered_set<FaceControl*>::iterator begin() { return _fcs.begin();}
    std::unordered_set<FaceControl*>::const_iterator begin() const { return _fcs.begin();}
    std::unordered_set<FaceControl*>::iterator end() { return _fcs.end();}
    std::unordered_set<FaceControl*>::const_iterator end() const { return _fcs.end();}

    // Given a prop, returns the associated FaceControl where FaceControl::belongs returns true or NULL if not present.
    // Currently does a linear lookup. TODO make hashable.
    FaceControl* find( const vtkProp*) const;

    // Convenience function to return the set of models (FaceControl::data) from the set.
    FaceModelSet models() const;

    // Convenience function to return the set of viewers.
    FaceViewerSet viewers() const;

private:
    std::unordered_set<FaceControl*> _fcs;
};  // end class


}   // end namespace
    
#endif
