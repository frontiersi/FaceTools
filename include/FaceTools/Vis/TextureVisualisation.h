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

#ifndef FACE_TOOLS_VIS_TEXTURE_VISUALISATION_H
#define FACE_TOOLS_VIS_TEXTURE_VISUALISATION_H

#include "BaseVisualisation.h"

namespace FaceTools { namespace Vis {

class TextureVisualisation : public BaseVisualisation
{
public:
    void setVisible( FV *fv, bool v) override { fv->setActiveColours(nullptr); fv->setTextured(v);}
    bool isVisible( const FV *fv) const override { return fv && fv->textured();}
    bool isAvailable( const FV *fv) const override
    {
        return fv && (fv->activeColours() || fv->canTexture());
    }   // end isAvailable
};  // end class

}}   // end namespaces

#endif
