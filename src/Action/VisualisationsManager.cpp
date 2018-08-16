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

#include <VisualisationsManager.h>
#include <FaceActionManager.h>
#include <ActionVisualise.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::VisualisationsManager;
using FaceTools::Action::FaceActionManager;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::FaceView;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceViewerSet;
using FaceTools::FaceModelSet;


VisualisationsManager::VisualisationsManager()
    : QObject(), _evis(this), _nvis(this)
{
    _nvis.setExclusive(false);
}   // end ctor


void VisualisationsManager::makeDefault( FaceActionManager* fman)
{
    fman->addAction( new ActionVisualise( &_tvis));
    fman->addAction( new ActionVisualise( &_svis));
    fman->addAction( new ActionVisualise( &_wvis));
    fman->addAction( new ActionVisualise( &_pvis));
}   // end makeDefault


void VisualisationsManager::add( FaceAction* a)
{
    ActionVisualise* av = qobject_cast<ActionVisualise*>(a);
    if ( av && av->isVisible() && av->manageVisualisation())
    {
        if ( av->isExclusive())
            _evis.addAction(av->qaction());
        else
            _nvis.addAction(av->qaction());
    }   // end if
}   // end add


QList<QAction*> VisualisationsManager::actions() const
{
    QList<QAction*> alist = _evis.actions();
    if ( !_nvis.actions().empty())
    {
        QAction* separator = new QAction;
        separator->setSeparator(true);
        alist.append( separator);
        alist.append( _nvis.actions());
    }   // end if
    return alist;
}   // end actions


bool VisualisationsManager::setDefaultVisualisation( FaceView* fv)
{
    //std::cerr << "VisMan::setDefaultVisualisation" << std::endl;
    bool setv = false;
    if ( fv->textureActor() != nullptr) // Try to apply the texture visualisation first
    {
        setv = fv->apply( &_tvis);
        assert(setv);
    }   // end if
    else if ( fv->surfaceActor() != nullptr)
    {
        setv = fv->apply( &_svis);
        assert(setv);
    }   // end else if
    return setv;
}   // end setDefaultVisualisation



void VisualisationsManager::enforceVisualisationConformance( const FaceControlSet* fcs)
{
    if ( !fcs)
        return;

    // Check for default visualisations first.
    const FaceModelSet& fms = fcs->models();
    for ( FaceModel* fm : fms)
    {
        for ( FaceControl* fc : fm->faceControls())
        {
            if ( fc->view()->exclusiveVisualisation() == nullptr)
                setDefaultVisualisation(fc->view());
        }   // end for
    }   // end for

    // Across all viewers possibly affected, enforce the same visualisation.
    FaceViewerSet viewers = fcs->viewers();
    for ( FaceModelViewer* viewer : viewers)
    {
        const FaceControlSet& vfcs = viewer->attached();
        assert( !vfcs.empty());
        BaseVisualisation *vis = nullptr;
        for ( FaceControl* fc : vfcs)
        {
            vis = fc->view()->exclusiveVisualisation();    // All must have this visualisation.
            if ( vis)   // Ensure no null visualisations selected as default
                break;
        }   // end for

        assert(vis);

        for ( FaceControl* fc : vfcs)
        {
            // If any of the views can't have this visualisation, all views in the viewer are set to the default.
            if ( fc->view()->exclusiveVisualisation() != vis)
            {
                if ( vis->isAvailable(fc))
                    fc->view()->apply( vis);
                else
                {
                    for ( FaceControl* fc : vfcs)
                        setDefaultVisualisation( fc->view());
                    break;
                }   // end else
            }   // end if
        }   // end for
    }   // end for
}   // end enforceVisualisationConformance
