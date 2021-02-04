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

#ifndef FACE_TOOLS_COPY_VIEW_H
#define FACE_TOOLS_COPY_VIEW_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionCopyView : public FaceAction
{ Q_OBJECT
public:
    // If a source viewer is set (can be null), this action will only be enabled for
    // the subset of the selected Vis::FVs that are in the source viewer.
    ActionCopyView( FMV* target, FMV* source, const QString&, const QIcon&);

    QString toolTip() const override { return "Copy the selected model's view into an adjacent viewer so that multiple visualisations can be shown concurrently.";}

protected:
    bool isAllowed( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    FMV *_tviewer;
    FMV *_sviewer;
};  // end class

}}   // end namespace

#endif
