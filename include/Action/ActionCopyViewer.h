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

#ifndef FACE_TOOLS_COPY_VIEWER_H
#define FACE_TOOLS_COPY_VIEWER_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionCopyViewer : public FaceAction
{ Q_OBJECT
public:
    // If a source viewer is set (can be NULL), this action will only be enabled for
    // the subset of the selected Vis::FVs that are in the source viewer.
    ActionCopyViewer( FMV* target, FMV* source, const QString&, const QIcon&);

protected slots:
    bool testReady( const Vis::FV* fc) override;
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet& cs, const FVS&, bool) override { cs.insert(VIEWER_CHANGE);}

private:
    FMV *_tviewer;
    FMV *_sviewer;
};  // end class

}}   // end namespace

#endif
