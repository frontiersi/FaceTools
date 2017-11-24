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
FaceModelTabWidget::FaceModelTabWidget( const QList<QAction*>* actions, QWidget *parent)
    : QTabWidget(parent),
      _actions(actions),
      _qvtkviewer( new QTools::VtkActorViewer),
      _viewer(NULL),
      _viewerLayout(NULL)
{
    _viewer = new FaceTools::InteractiveModelViewer( _qvtkviewer);
    _viewerLayout = new QVBoxLayout();
    _viewerLayout->setContentsMargins( QMargins());
    _viewer->addToLayout( _viewerLayout);

    setUsesScrollButtons(true);
    setDocumentMode(true);
    connect( this, &FaceModelTabWidget::currentChanged, this, &FaceModelTabWidget::onTabChanged);
    connect( _viewer, &FaceTools::InteractiveModelViewer::requestContextMenu, this, &FaceModelTabWidget::requestContextMenu);
}   // end ctor


// public
FaceModelTabWidget::~FaceModelTabWidget()
{
    for ( int i = 0; i < count(); ++i)
        removeTabWidget( i);
    delete _viewerLayout;
    delete _viewer;
    delete _qvtkviewer;
}   // end dtor


// public
int FaceModelTabWidget::addTabWidget( FaceModel* fmodel)
{
    // Create the widget and add the actions
    FaceModelWidget* fmwidget = new FaceModelWidget( _viewer, _actions);
    connect( fmwidget, &FaceModelWidget::onMadeActive, this, &FaceModelTabWidget::onMadeActive);
    const std::string& tname = fmwidget->addView( fmodel);
    setUpdatesEnabled(false);  // Disable painting (prevent flicker)
    const int tabIdx = addTab( fmwidget, tname.c_str());
    setUpdatesEnabled(true);   // Enable painting again
    setCurrentIndex(tabIdx);
    return tabIdx;
}   // end addTabWidget


// public
size_t FaceModelTabWidget::removeModel( FaceModel* fmodel)
{
    size_t nViewsRem = 0;
    QList<int> remTabs;
    for ( int tabIdx = 0; tabIdx < count(); ++tabIdx)
    {
        FaceModelWidget* fmwidget = qobject_cast<FaceModelWidget*>( widget(tabIdx));
        nViewsRem += fmwidget->removeModel(fmodel);
        if ( fmwidget->getNumViews() == 0)
            remTabs << tabIdx;
    }   // end foreach

    setUpdatesEnabled(false);  // Disable painting (prevent flicker)
    // Remove tab widgets that no longer have views in them
    foreach ( int tabIdx, remTabs)
        removeTabWidget(tabIdx);

    onTabChanged( currentIndex());
    setUpdatesEnabled(true);   // Enable painting again
    return nViewsRem;
}   // end removeModel


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
    removeTab(tabIdx);
    delete fmwidget;
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
void FaceModelTabWidget::onTabChanged( int tabIdx)
{
    std::string vname;
    Mint* mint = NULL;
    if ( tabIdx < 0)
        setLayout(_viewerLayout);
    else
    {
        FaceModelWidget* fmwidget = qobject_cast<FaceModelWidget*>( widget(tabIdx));
        mint = fmwidget->getActive( &vname);
        fmwidget->reparentViewer(_viewerLayout);
    }   // end else
    emit onMadeActive( mint, vname);
}   // end onTabChanged
