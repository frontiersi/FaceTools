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

#include <ActionClean.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <ObjModelIntegrityChecker.h>
#include <QMessageBox>
using FaceTools::Action::ActionClean;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionClean::ActionClean( QWidget* parent, QProgressBar* pb)
    : FaceAction(true/*disable before other*/), _icon( ":/icons/CLEAN"), _parent(parent)
{
    addRespondTo( MODEL_GEOMETRY_CHANGED);
    addChangeTo( MODEL_GEOMETRY_CHANGED);
    if ( pb)
        setAsync( true, QTools::QProgressUpdater::create(pb));
}   // end ctor


ActionClean::~ActionClean()
{
    _clean.clear();
}   // end dtor


bool ActionClean::testReady( FaceControl* fc)
{
    if ( _clean.count(fc->data()) == 0)
        checkClean(fc->data());
    return !_clean.at(fc->data());
}   // end testReady


bool ActionClean::doAction( FaceControlSet& rset)
{
    const FaceModelSet& fms = rset.models();
    for ( FaceModel* fm : fms)
    {
        FaceTools::clean( fm->model());
        fm->setModel(fm->model());
        if ( !checkClean(fm))
            _failset.insert(fm);
    }   // end for
    return _failset.empty();
}   // end doAction


void ActionClean::doAfterAction( const FaceControlSet&, bool v)
{
    if ( !_failset.empty())
        showCleaningError();
    _failset.clear();
}   // end doAfterAction


void ActionClean::respondToChange( FaceControl* fc)
{
    if ( _clean.count(fc->data()) > 0)
    {
        bool clean = checkClean(fc->data());
        if ( !clean)
            clean = process(fc);
        if ( !clean)
            showCleaningError();
    }   // end if
    FaceAction::respondToChange(fc);    // Calls testReady
}   // end respondToChange


void ActionClean::burn( const FaceControl* fc)
{
    if ( _clean.count(fc->data()) > 0)
        _clean.erase(fc->data());
}   // end burn


bool ActionClean::checkClean( const FaceModel* fm)
{
    RFeatures::ObjModelIntegrityChecker ic(fm->model());
    _clean[fm] = ic.checkIntegrity();
    return _clean.at(fm);
}   // end checkClean


void ActionClean::showCleaningError() const
{
    QMessageBox::critical( _parent, tr("Unable to clean!"), tr("Couldn't clean the provided model(s) to be a triangulated mesh!"));
}   // end showCleaningError
