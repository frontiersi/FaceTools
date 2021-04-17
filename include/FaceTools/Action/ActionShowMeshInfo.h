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

#ifndef FACE_TOOLS_ACTION_SHOW_MESH_INFO_H
#define FACE_TOOLS_ACTION_SHOW_MESH_INFO_H

#include "ActionDiscardManifold.h"
#include "ActionRemoveManifolds.h"
#include <FaceTools/Widget/MeshInfoDialog.h>
#include <FaceTools/Vis/OutlinesVisualisation.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionShowMeshInfo : public FaceAction
{ Q_OBJECT
public:
    ActionShowMeshInfo( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Show information about the selected 3D mesh.";}

    void setDiscardManifoldAction( ActionDiscardManifold*);
    void setRemoveManifoldsAction( ActionRemoveManifolds*);

protected:
    void postInit() override;
    bool update( Event) override;
    bool isAllowed( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;
    void purge( const FM*) override;

private slots:
    void _doOnSelectedManifoldChanged( int);
    void _doOnDialogClosed();

private:
    Widget::MeshInfoDialog *_dialog;
    ActionDiscardManifold *_dmact;
    ActionRemoveManifolds *_rmact;
    Vis::OutlinesVisualisation _vis;
};  // end class

}}   // end namespace

#endif
