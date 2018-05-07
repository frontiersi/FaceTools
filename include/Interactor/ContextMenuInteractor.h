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

#ifndef FACE_TOOLS_CONTEXT_MENU_INTERACTOR_H
#define FACE_TOOLS_CONTEXT_MENU_INTERACTOR_H

#include "ModelViewerInteractor.h"
#include <QMenu>

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT ContextMenuInteractor : public ModelViewerInteractor
{ Q_OBJECT
public:
    explicit ContextMenuInteractor( QMenu* cm=NULL);

    void setContextMenu( QMenu*);

private:
    bool rightButtonDown( const QPoint&) override;
    QMenu *_cmenu;
};  // end class

}   // end namespace
}   // end namespace

#endif
