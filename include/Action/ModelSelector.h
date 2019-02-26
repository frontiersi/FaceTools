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

#ifndef FACE_TOOLS_MODEL_SELECTOR_H
#define FACE_TOOLS_MODEL_SELECTOR_H

#include <FaceModelViewer.h>
#include <ModelSelectInteractor.h>

namespace FaceTools { namespace Action {

class FaceActionManager;

class FaceTools_EXPORT ModelSelector
{
public:
    using Ptr = std::shared_ptr<ModelSelector>;

    static void create( FaceActionManager*, FMV*);

    static FMV* viewer();   // The currently selected viewer
    static Vis::FV* selected() { return _me->_msi.selected();}
    static const FVS& available() { return _me->_msi.available();}

    // Create a new FaceView instances and attach it to the given viewer.
    // If given viewer is null, FaceView added to currently selected viewer.
    // Returned FaceView is NOT automatically selected.
    static Vis::FV* addFaceView( FM*, FMV *v=nullptr);

    // Programmatically select/deselect the given FaceView.
    static void setSelected( Vis::FV*, bool);

    static void setSelectEnabled( bool);

    // Detach the FaceView from its viewer and delete it.
    static void removeFaceView( Vis::FV*);

    // Call removeFaceView for ALL associated FaceViews of the given FaceModel.
    static void remove( FM*);

private:
    static ModelSelector::Ptr _me;
    Interactor::ModelSelectInteractor _msi;
    ModelSelector( FaceActionManager*, FMV*);
};  // end class

}}   // end namespace

#endif
