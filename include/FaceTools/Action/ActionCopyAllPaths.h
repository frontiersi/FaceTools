/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_COPY_ALL_PATHS_H
#define FACE_TOOLS_ACTION_COPY_ALL_PATHS_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionCopyAllPaths : public FaceAction
{ Q_OBJECT
public:
    ActionCopyAllPaths( const QString&, const QIcon&);

    QString toolTip() const override;
    QString whatsThis() const override;

    // Returns true iff the user measurements from src can be copied
    // to dst because they both have the same anthropometric mask.
    static bool canCopy( const FM *src, const FM *dst);

    // Copy the user measurements from src to dst returning the number copied over.
    static size_t copy( const FM &src, FM &dst);

protected:
    bool isAllowed( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;
};  // end class

}}   // end namespace

#endif
