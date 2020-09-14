/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_SELECT_NOTIFIER_H
#define FACE_TOOLS_SELECT_NOTIFIER_H

#include <FaceTools/FaceViewSet.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT SelectNotifier : public QObject
{ Q_OBJECT
public:
    SelectNotifier();

    void add( Vis::FV*);
    void remove( Vis::FV*); // Removes from available for selection and fires onSelected if FV was selected.
    void setSelected( Vis::FV*, bool);  // Mark given model as (de)selected and fire onSelected.
    Vis::FV* selected() const { return _selected;}

    // Locking the notifier prevents a different model from being selected
    // and thus also prevents signal onSelected from being emitted.
    void setLocked( bool v) { _locked = v;}
    bool isLocked() const { return _locked;}

signals:
    void onSelected( Vis::FV*, bool) const;   // Notify of model (de)selection.

private:
    bool _locked;
    Vis::FV* _selected;
    FVS _available;

    SelectNotifier( const SelectNotifier&) = delete;
    void operator=( const SelectNotifier&) = delete;
};  // end class

}}   // end namespace

#endif
