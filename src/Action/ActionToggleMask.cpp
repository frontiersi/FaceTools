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

#include <Action/ActionToggleMask.h>
#include <Vis/MaskVisualisation.h>
using FaceTools::Action::ActionToggleMask;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::Event;


ActionToggleMask::ActionToggleMask( const QString& dn, const QIcon& ico, const QKeySequence &ks)
    : ActionVisualise( dn, ico, new Vis::MaskVisualisation, ks)
{
    addPurgeEvent(Event::MASK_CHANGE);
}   // end ctor


QString ActionToggleMask::toolTip() const
{
    return "Toggle view between the original and its coregistered mask.";
}   // end toolTip


QString ActionToggleMask::whatsThis() const
{
    QStringList htxt;
    htxt << "If this model has a correspondence mask defined, this toggles the view";
    htxt << "between the original model, and the correspondence mask. Note that";
    htxt << "texture mapping is not defined on the correspondence mask.";
    return htxt.join(" ");
}   // end whatsThis
