/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionShowPhenotypes.h>
using FaceTools::Action::ActionShowPhenotypes;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Widget::PhenotypesDialog;


ActionShowPhenotypes::ActionShowPhenotypes( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _dialog(nullptr)
{
    setCheckable( true, false);
}   // end ctor


bool ActionShowPhenotypes::update( Event)
{
    return _dialog->isVisible();
}   // end update


void ActionShowPhenotypes::postInit()
{
    _dialog = new PhenotypesDialog( static_cast<QWidget*>(parent()));
    connect( _dialog, &PhenotypesDialog::accepted, [this](){ setChecked(false);});
}   // end postInit


void ActionShowPhenotypes::doAction( Event)
{
    if ( isChecked())
        _dialog->show();
}   // end doAction

