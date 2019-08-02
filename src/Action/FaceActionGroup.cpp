/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <FaceActionGroup.h>
#include <algorithm>
using FaceTools::Action::FaceActionGroup;
using FaceTools::Action::FaceAction;


// public
FaceActionGroup::FaceActionGroup() : _waction(nullptr)
{
}   // end ctor


// protected
bool FaceActionGroup::addAction( FaceAction* faction)
{
    const std::string nm = faction->displayName().toStdString();
    if ( _actions.count(nm) > 0)
        return false;

    _actions[nm] = faction;
    _alist << faction;
    return true;
}   // end addAction


// public virtual
QStringList FaceActionGroup::interfaceIds() const
{
    QStringList qlist;
    for ( const FaceAction* fa : _alist)
        qlist << fa->displayName();
    return qlist;
}   // end interfaceIds


// public virtual
FaceAction* FaceActionGroup::iface( const QString& iname) const
{
    const std::string nm = iname.toStdString();
    if ( _actions.count(nm) == 0)
        return nullptr;
    return _actions.at(nm);
}   // end iface
