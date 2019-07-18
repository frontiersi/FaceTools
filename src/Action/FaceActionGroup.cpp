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


namespace {
void addToWidget( const QList<FaceAction*>& actions, QWidget* widget)
{
    for ( FaceAction* fa : actions)
        widget->addAction( fa->qaction());
}   // end addToWidget
}   // end namespace


// public virtual
QMenu* FaceActionGroup::createMenu() const
{
    QMenu* menu = new QMenu( displayName(), nullptr);
    addToWidget( _alist, menu);
    return menu;
}   // end createMenu


// public virtual
QToolBar* FaceActionGroup::createToolBar() const
{
    QToolBar* toolbar = new QToolBar( displayName(), nullptr);
    addToWidget( _alist, toolbar);
    return toolbar;
}   // end createToolBar


// public
QAction* FaceActionGroup::qaction()
{
    QWidget* w = widget();
    if ( w && !_waction)
    {   // Widget has been overridden in a derived type, so create a default action to show it.
        _waction = new QAction(nullptr);
        const QIcon* ic = icon();
        if ( ic)
            _waction->setIcon( *ic);
        _waction->setText( displayName()); // getDisplayName is class name by default!
        const QKeySequence* keys = shortcut();
        if ( keys)
            _waction->setShortcut(*keys);
        connect( _waction, &QAction::triggered, w, &QWidget::show);
        connect( _waction, &QAction::triggered, w, &QWidget::raise);
    }   // end if
    return _waction;
}   // end qaction


// public virtual
void FaceActionGroup::addTo( QMenu* menu) const
{
    if ( !menu->actions().empty())
        menu->addSeparator();
    addToWidget( _alist, menu);
}   // end addTo


// public virtual
void FaceActionGroup::addTo( QToolBar* toolbar) const
{
    if ( !toolbar->actions().empty())
        toolbar->addSeparator();
    addToWidget( _alist, toolbar);
}   // end addTo


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
