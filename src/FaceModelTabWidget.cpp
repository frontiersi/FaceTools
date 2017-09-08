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

#include <FaceModelTabWidget.h>
#include <FaceActionInterface.h>
using FaceTools::FaceModelTabWidget;
using FaceTools::FaceModelWidget;
using FaceTools::FaceAction;
using FaceTools::FaceModel;


// public
FaceModelTabWidget::FaceModelTabWidget( QWidget *parent)
    : QTabWidget(parent),
     _viewer( new FaceTools::InteractiveModelViewer( true))
{
    setUsesScrollButtons(true);
    setDocumentMode(true);
    connect( _viewer, &FaceTools::InteractiveModelViewer::requestContextMenu,
                this, &FaceModelTabWidget::showContextMenu);
}   // end ctor


// public
FaceModelTabWidget::~FaceModelTabWidget()
{
    for ( int i = 0; i < count(); ++i)
        removeTabWidget( i);
    delete _viewer;
}   // end dtor


// public
bool FaceModelTabWidget::addAction( QAction* action)
{
    assert(action);
    FaceAction* faction = qobject_cast<FaceAction*>(action->parent());
    if ( !faction)
    {
        std::cerr << "[ERROR] FaceTools::FaceModelTabWidget::addAction: QAction is not parented by a FaceAction!" << std::endl;
        return false;
    }   // end if
    _xactions << action;
    return true;
}   // end addAction


// public
bool FaceModelTabWidget::addContextActionGroup( QActionGroup* ag)
{
    foreach ( QAction* action, ag->actions())
    {
        if ( !addAction(action))
            return false;
    }   // end foreach

    if ( !_cmenu.actions().empty())
        _cmenu.addSeparator();
    _cmenu.addActions( ag->actions());
    return true;
}   // end addContextActionGroup


// public
int FaceModelTabWidget::addTabWidget( FaceModel* fmodel)
{
    // Create the widget and add the actions
    FaceModelWidget* fmwidget = new FaceModelWidget( _viewer, &_xactions);
    connect( fmwidget, &FaceModelWidget::onViewSelected, this, &FaceModelTabWidget::activated);
    const std::string& tname = fmwidget->addView( fmodel);
    setUpdatesEnabled(false);  // Disable painting (prevent flicker)
    const int tabIdx = addTab( fmwidget, tname.c_str());
    setUpdatesEnabled(true);   // Enable painting again
    setCurrentIndex(tabIdx);
    return tabIdx;
}   // end addTabWidget


// public
void FaceModelTabWidget::addViewToTab( int tabIdx, FaceModel* fmodel)
{
    FaceModelWidget* fmwidget = qobject_cast<FaceModelWidget*>( widget(tabIdx));
    const std::string& tname = fmwidget->addView( fmodel);
    setTabText( tabIdx, tname.c_str());
}   // end addViewToTab


// public
size_t FaceModelTabWidget::removeModelFromTab( int tabIdx, FaceModel* fmodel)
{
    FaceModelWidget* fmwidget = qobject_cast<FaceModelWidget*>( widget(tabIdx));
    return fmwidget->removeModel(fmodel);
}   // end removeModelFromTab


// public
size_t FaceModelTabWidget::removeTabWidget( int tabIdx)
{
    FaceModelWidget* fmwidget = qobject_cast<FaceModelWidget*>( widget(tabIdx));
    const size_t nmodels = fmwidget->getNumModels();
    setUpdatesEnabled(false);  // Disable painting (prevent flicker)
    fmwidget->disconnect(this);
    removeTab(tabIdx);         // Causes the widget to be deleted
    setUpdatesEnabled(true);   // Enable painting again
    return nmodels;
}   // end removeTabWidget


// public
void FaceModelTabWidget::consolidateTabs()
{
    // Get all the existing models from the specified tabs. Note that only models are consolidated - not views!
    boost::unordered_set<FaceModel*> fmodels;
    for ( int tabIdx = 0; tabIdx < count(); ++tabIdx)
    {
        qobject_cast<FaceModelWidget*>( widget(tabIdx))->getModels(fmodels);
        removeTabWidget(tabIdx);    // Remove this tab
    }   // end foreach

    if ( fmodels.empty())
        return;

    // Create a new tab, and add all of the models
    boost::unordered_set<FaceModel*>::const_iterator it = fmodels.begin();
    int tabIdx = addTabWidget(*it);
    while ( ++it != fmodels.end())
        addViewToTab( tabIdx, *it);
    setCurrentIndex( tabIdx);
}   // end consolidateTabs


// private slot
void FaceModelTabWidget::showContextMenu( const QPoint& p)
{
    if ( count() == 0 || _cmenu.actions().empty())
        return;

    const int tabIdx = currentIndex();
    const FaceView* fview = qobject_cast<FaceModelWidget*>( widget(tabIdx))->getActiveView();
    if ( fview && fview->isPointedAt(p))
        _cmenu.exec(p);
}   // end showContextMenu
