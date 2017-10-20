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

#include <ActionAddLandmark.h>
#include <ModelInteractor.h>
#include <FaceModel.h>
#include <ObjMetaData.h>
#include <QInputDialog>
#include <QMessageBox>
#include <cassert>
using FaceTools::ActionAddLandmark;
using FaceTools::ModelInteractor;

ActionAddLandmark::ActionAddLandmark( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon( fname.c_str()),
      _interactor(NULL)
{
    init();
    checkEnable();
}   // end ctor


void ActionAddLandmark::setInteractive( ModelInteractor* interactor, bool enable)
{
    _interactor = NULL;
    if ( !enable)
        interactor->disconnect( this);
    else
    {
        _interactor = interactor;
        connect( _interactor, &ModelInteractor::onEnteringLandmark, this, &ActionAddLandmark::checkEnable);
        connect( _interactor, &ModelInteractor::onExitingLandmark, this, &ActionAddLandmark::checkEnable);
        connect( _interactor, &ModelInteractor::onEnteringModel, this, &ActionAddLandmark::checkEnable);
        connect( _interactor, &ModelInteractor::onExitingModel, this, &ActionAddLandmark::checkEnable);
    }   // end else
    checkEnable();
}   // end setInteractive


bool ActionAddLandmark::doAction()
{
    assert(_interactor);
    FaceTools::FaceModel* fmodel = _interactor->getModel();

    // Get a new name for the landmark that isn't one of the existing ones!
    FaceTools::ObjMetaData::Ptr objmeta = fmodel->getObjectMeta();
    boost::unordered_set<std::string> lmnames;
    objmeta->getLandmarks( lmnames);

    // Get new landmark name from user
    bool ok = false;
    std::string newname;
    do
    {
        const QString qname = QInputDialog::getText( NULL, tr("Add new landmark"), tr("Name of new landmark:"),
                                                     QLineEdit::Normal, "", &ok);
        newname = qname.toStdString();
        if ( lmnames.count(newname) > 0)
        {
            ok = false;
            QMessageBox mb( tr("Duplicate Name"), tr(("Landmark '" + newname + "' already exists! Try again?").c_str()),
                            QMessageBox::Information,
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No | QMessageBox::Escape,
                            QMessageBox::NoButton);
            if ( mb.exec() == QMessageBox::No)
                newname = "";
        }   // end if
    } while ( !ok && !newname.empty());

    bool added = false;
    if ( ok && !newname.empty())
    {
        added = true;
        cv::Vec3f v;
        const bool onModel = _interactor->calcSurfacePosition(v);
        assert(onModel);
        fmodel->updateLandmark( newname, &v);
    }   // end if
    return added;
}   // end doAction


// private slot
void ActionAddLandmark::checkEnable()
{
    if ( !_interactor)
        setEnabled(false);
    else
    {
        // Allow landmark add if pointing at model but not an existing landmark AND user isn't drawing a path.
        setEnabled(  _interactor->getView()->isPointedAt( _interactor->getMouseCoords()) && !_interactor->isDrawingPath());
    }   // end else
}   // end checkEnable


