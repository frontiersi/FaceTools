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

#include <MultiFaceModelViewer.h>
#include <FeatureUtils.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <QImageTools.h>
#include <QVBoxLayout>
#include <QAction>
#include <QIcon>
#include <cassert>
#include <algorithm>
using FaceTools::MultiFaceModelViewer;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;

namespace {
void addButton( QLayout* l, QAction* a)
{
    QToolButton* b = new QToolButton;
    b->setDefaultAction(a);
    b->setIconSize( QSize(22,22));
    b->setMinimumSize( QSize(22,22));
    b->setMaximumSize( QSize(22,22));
    b->setStyleSheet( "QToolButton::menu-indicator { image: none; }");
    l->addWidget(b);
}   // end addButton
}   // end namespace


// private
void MultiFaceModelViewer::addCommonButtons( QLayout* l, FaceModelViewer* viewer)
{
    QAction* a0 = new QAction( QIcon(":/icons/SCREENSHOT"), tr("Save Screenshot"), this);
    connect( a0, &QAction::triggered, viewer, &FaceModelViewer::saveScreenshot);
    QAction* a1 = new QAction( QIcon(":/icons/RESET_CAMERA"), tr("Reset Camera"), this);
    connect( a1, &QAction::triggered, viewer, &FaceModelViewer::resetCamera);
    addButton( l, a0);
    addButton( l, a1);
}   // end addCommonButtons


// private
QToolButton* MultiFaceModelViewer::makeButton( QAction* action)
{
    QToolButton* b = new QToolButton;
    b->setDefaultAction(action);
    b->setMinimumSize( QSize(70,22));
    b->setMaximumSize( QSize(70,22));
    b->setIconSize( QSize(22,22));
    b->setStyleSheet( "QToolButton::menu-indicator { image: none; }");
    return b;
}   // makeButton


// public
MultiFaceModelViewer::MultiFaceModelViewer( QWidget *parent) : QWidget(parent)
{
    _v0 = new FaceModelViewer( this); // Left
    _v1 = new FaceModelViewer( this); // Middle
    _v2 = new FaceModelViewer( this); // Right

    QVBoxLayout* v0layout = new QVBoxLayout;
    v0layout->setContentsMargins(0,0,0,0);
    QVBoxLayout* v1layout = new QVBoxLayout;
    v1layout->setContentsMargins(0,0,0,0);
    QVBoxLayout* v2layout = new QVBoxLayout;
    v2layout->setContentsMargins(0,0,0,0);

    QIcon goLeftIcon( ":/icons/MOVE_LEFT");
    QIcon goRightIcon( ":/icons/MOVE_RIGHT");
    QIcon copyLeftIcon( ":/icons/COPY_LEFT");
    QIcon copyRightIcon( ":/icons/COPY_RIGHT");

    // Left panel actions
    _moveLeftToCentreAction = new QAction( goRightIcon, "Move Face(s) Right", this);
    connect( _moveLeftToCentreAction, &QAction::triggered, this, &MultiFaceModelViewer::moveLeftToCentre);
    _copyLeftToCentreAction = new QAction( copyRightIcon, "Copy Face(s) Right", this);
    connect( _copyLeftToCentreAction, &QAction::triggered, this, &MultiFaceModelViewer::copyLeftToCentre);

    // Centre panel actions
    _moveCentreToRightAction = new QAction( goRightIcon, "Move Face(s) Right", this);
    connect( _moveCentreToRightAction, &QAction::triggered, this, &MultiFaceModelViewer::moveCentreToRight);
    _copyCentreToRightAction = new QAction( copyRightIcon, "Copy Face(s) Right", this);
    connect( _copyCentreToRightAction, &QAction::triggered, this, &MultiFaceModelViewer::copyCentreToRight);
    _moveCentreToLeftAction = new QAction( goLeftIcon, "Move Face(s) Left", this);
    connect( _moveCentreToLeftAction, &QAction::triggered, this, &MultiFaceModelViewer::moveCentreToLeft);
    _copyCentreToLeftAction = new QAction( copyLeftIcon, "Copy Face(s) Left", this);
    connect( _copyCentreToLeftAction, &QAction::triggered, this, &MultiFaceModelViewer::copyCentreToLeft);

    // Right panel actions
    _moveRightToCentreAction = new QAction( goLeftIcon, "Move Face(s) Left", this);
    connect( _moveRightToCentreAction, &QAction::triggered, this, &MultiFaceModelViewer::moveRightToCentre);
    _copyRightToCentreAction = new QAction( copyLeftIcon, "Copy Face(s) Left", this);
    connect( _copyRightToCentreAction, &QAction::triggered, this, &MultiFaceModelViewer::copyRightToCentre);

    // Left panel
    v0layout->addWidget(_v0);
    QHBoxLayout* h0blayout = new QHBoxLayout;
    h0blayout->setContentsMargins(0,0,0,0);
    addCommonButtons( h0blayout, _v0);
    h0blayout->addStretch();
    h0blayout->addWidget( makeButton( _copyLeftToCentreAction));
    h0blayout->addWidget( makeButton( _moveLeftToCentreAction));
    v0layout->addLayout(h0blayout);

    // Centre panel
    v1layout->addWidget(_v1);
    QHBoxLayout* h1blayout = new QHBoxLayout;
    h1blayout->setContentsMargins(0,0,0,0);
    h1blayout->addWidget( makeButton( _moveCentreToLeftAction));
    h1blayout->addWidget( makeButton( _copyCentreToLeftAction));
    h1blayout->addStretch();
    addCommonButtons( h1blayout, _v1);
    h1blayout->addStretch();
    h1blayout->addWidget( makeButton( _copyCentreToRightAction));
    h1blayout->addWidget( makeButton( _moveCentreToRightAction));
    v1layout->addLayout( h1blayout);

    // Right panel
    v2layout->addWidget(_v2);
    QHBoxLayout* h2blayout = new QHBoxLayout;
    h2blayout->setContentsMargins(0,0,0,0);
    h2blayout->addWidget( makeButton( _moveRightToCentreAction));
    h2blayout->addWidget( makeButton( _copyRightToCentreAction));
    h2blayout->addStretch();
    addCommonButtons( h2blayout, _v2);
    v2layout->addLayout(h2blayout);

    QWidget* w0 = new QWidget;
    w0->setLayout(v0layout);
    QWidget* w1 = new QWidget;
    w1->setLayout(v1layout);
    QWidget* w2 = new QWidget;
    w2->setLayout(v2layout);

    _splitter = new QSplitter;
    _splitter->addWidget( w0);
    _splitter->addWidget( w1);
    _splitter->addWidget( w2);

    setLayout( new QVBoxLayout);
    layout()->addWidget( _splitter);

    connect( _v0, &FaceModelViewer::onUserSelected, this, &MultiFaceModelViewer::doOnLeftViewerSetSelected);
    connect( _v1, &FaceModelViewer::onUserSelected, this, &MultiFaceModelViewer::doOnCentreViewerSetSelected);
    connect( _v2, &FaceModelViewer::onUserSelected, this, &MultiFaceModelViewer::doOnRightViewerSetSelected);

    _splitter->setCollapsible(1,false);
    setLeftViewerVisible(false);
    setRightViewerVisible(false);
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
}   // end ctor


// public
MultiFaceModelViewer::~MultiFaceModelViewer()
{
    _v0->disconnect(this);
    _v1->disconnect(this);
    _v2->disconnect(this);
    std::unordered_set<FaceModel*> fmodels; // Get all the FaceModels to remove...
    std::for_each( std::begin(_fconts), std::end(_fconts), [&](FaceControl* fc){fmodels.insert(fc->data());});
    // ... remove the models
    std::for_each( std::begin(fmodels), std::end(fmodels), [this](FaceModel* fm){remove(fm);});
}   // end dtor


// private
bool MultiFaceModelViewer::deleteFaceControl( FaceControl* fc)
{
    if ( !fc)
        return false;

    FaceModel* fm = fc->data();
    FaceModelViewer* v = fc->viewer();
    activateOnViewer( v, fc, false);    // Will cause onSelected(fc, false) to fire
    emit onRemoving( fc);   // Allow others to remove themselves BEFORE the viewer is detached.
    v->detach( fc);     // Sets viewer NULL in the FaceControl
    delete fc;
    fm->remove(fc);
    _fconts.erase(fc);
    v->updateRender();
    if ( v == _v0)
        setLeftViewerVisible( _v0->count() > 0);
    else if ( v == _v2)
        setRightViewerVisible( _v2->count() > 0);
    return true;
}   // end deleteFaceControl


// public slot
void MultiFaceModelViewer::remove( FaceModel* fmodel)
{
    deleteFaceControl( _v0->get(fmodel));
    deleteFaceControl( _v1->get(fmodel));
    deleteFaceControl( _v2->get(fmodel));
}   // end remove


// public slot
void MultiFaceModelViewer::insert( FaceModel* fmodel)
{
    FaceControl* fcont = new FaceControl(fmodel);
    fmodel->add(fcont);
    _fconts.insert( fcont);
    _v1->attach( fcont);
    activateOnViewer( _v1, fcont, true);
}   // end insert


// public slot
void MultiFaceModelViewer::setLeftViewerVisible(bool visible)
{
    QList<int> widths = _splitter->sizes();
    int sum = widths[0] + widths[1] + widths[2];
    if ( visible && widths[0] == 0)
        widths[0] = (int)(double(widths[1])/2);
    else if ( !visible && widths[0] > 0)
        widths[0] = 0;
    _splitter->setSizes( widths);
}   // end setLeftViewerVisible


// public slot
void MultiFaceModelViewer::setRightViewerVisible(bool visible)
{
    QList<int> widths = _splitter->sizes();
    int sum = widths[0] + widths[1] + widths[2];
    if ( visible && widths[2] == 0)
        widths[2] = (int)(double(widths[1])/2);
    else if ( !visible && widths[2] > 0)
        widths[2] = 0;
    _splitter->setSizes( widths);
}   // end setRightViewerVisible


// public
void MultiFaceModelViewer::saveScreenshot() const
{
    std::vector<cv::Mat> mimgs;
    if ( _splitter->sizes()[0] > 0)
        mimgs.push_back( _v0->grabImage());
    if ( _splitter->sizes()[1] > 0)
        mimgs.push_back( _v1->grabImage());
    if ( _splitter->sizes()[2] > 0)
        mimgs.push_back( _v2->grabImage());
    cv::Mat m = RFeatures::concatHorizontalMax( mimgs);
    QTools::saveImage( m);
}   // end saveScreenshot


// private
void MultiFaceModelViewer::moveViews( FaceModelViewer* sv, FaceModelViewer* tv)
{
    FaceControlSet fcs = sv->selected();    // Copy out
    for ( FaceControl* fc : fcs)
    {
        // If the target doesn't yet have the model, move it to the target viewer from the source viewer.
        if ( !tv->isAttached(fc))
        {
            sv->detach( fc);
            activateOnViewer( sv, fc, false);
            tv->attach( fc);
            activateOnViewer( tv, fc, true);
        }   // end if
        else    // Otherwise, this is a duplicate model on the source viewer, so delete it.
        {
            deleteFaceControl( fc);  // No longer needed on the source viewer
            tv->setSelected( fc, true);
            emit onSelected( fc, true);         // Inform clients
        }   // end else
    }   // end foreach
    sv->updateRender();
    tv->updateRender();
}   // end moveViews


// private
void MultiFaceModelViewer::copyViews( FaceModelViewer* sv, FaceModelViewer* tv)
{
    FaceControlSet fcs;
    canCopyTo( sv, tv, &fcs);
    for ( FaceControl* fc : fcs)
    {
        FaceControl* fcont = new FaceControl(fc->data());   // Create new view/control from source view/control's model
        fc->data()->add( fcont);
        _fconts.insert( fcont);
        tv->attach( fcont);
        activateOnViewer( tv, fcont, true);
    }   // end foreach
    tv->updateRender();
}   // end copyViews


// private slot
void MultiFaceModelViewer::moveLeftToCentre()
{
    moveViews( _v0, _v1);
    checkEnableLeftToCentre();
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    setLeftViewerVisible( _v0->count() > 0);
}   // end moveLeftToCentre


// private slot
void MultiFaceModelViewer::copyLeftToCentre()
{
    copyViews( _v0, _v1);
    checkEnableLeftToCentre();
    activateAllOnViewer( _v0, false);
}   // end copyLeftToCentre


// private slot
void MultiFaceModelViewer::moveCentreToLeft()
{
    moveViews( _v1, _v0);
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    checkEnableLeftToCentre();
    setLeftViewerVisible(true);
}   // end moveCentreToLeft


// private slot
void MultiFaceModelViewer::copyCentreToLeft()
{
    copyViews( _v1, _v0);
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    setLeftViewerVisible(true);
    activateAllOnViewer( _v1, false);
}   // end copyCentreToLeft


// private slot
void MultiFaceModelViewer::moveCentreToRight()
{
    moveViews( _v1, _v2);
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    checkEnableRightToCentre();
    setRightViewerVisible(true);
}   // end moveCentreToRight


// private slot
void MultiFaceModelViewer::copyCentreToRight()
{
    copyViews( _v1, _v2);
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    setRightViewerVisible(true);
    activateAllOnViewer( _v1, false);
}   // end copyCentreToRight


// private slot
void MultiFaceModelViewer::moveRightToCentre()
{
    moveViews( _v2, _v1);
    checkEnableRightToCentre();
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    setRightViewerVisible( _v2->count() > 0);
}   // moveRightToCentre


// private slot
void MultiFaceModelViewer::copyRightToCentre()
{
    copyViews( _v2, _v1);
    checkEnableRightToCentre();
    activateAllOnViewer( _v2, false);
}   // copyRightToCentre


// private
void MultiFaceModelViewer::activateAllOnViewer( FaceModelViewer* viewer, bool activate)
{
    const FaceControlSet* fcs = NULL;
    FaceControlSet sfcs;
    if ( activate)
        fcs = &viewer->attached();
    else
    {
        sfcs = viewer->selected();    // Copy out
        fcs = &sfcs;
    }   // end if

    std::for_each( std::begin(*fcs), std::end(*fcs), [&](FaceControl* fc){ activateOnViewer( viewer, fc, activate);});
}   // end activateAllOnViewer


// private
void MultiFaceModelViewer::activateOnViewer( FaceModelViewer* viewer, FaceControl* fc, bool controlled)
{
    viewer->setSelected( fc, controlled);
    if ( viewer == _v0)
        doOnLeftViewerSetSelected( fc, controlled);
    else if ( viewer == _v1)
        doOnCentreViewerSetSelected( fc, controlled);
    else
    {
        assert( viewer == _v2);
        doOnRightViewerSetSelected( fc, controlled);
    }   // end else
}   // end activateOnViewer


// private slot
void MultiFaceModelViewer::doOnLeftViewerSetSelected( FaceControl* fcont, bool v)
{
    checkEnableLeftToCentre();
    if ( v) // Ensure that models in the other viewers are not controlled.
    {
        activateAllOnViewer( _v1, false);
        activateAllOnViewer( _v2, false);
    }   // end if
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    checkEnableRightToCentre();
    emit onSelected( fcont, v);
}   // end doOnLeftViewerSetSelected


// private slot
void MultiFaceModelViewer::doOnCentreViewerSetSelected( FaceControl* fcont, bool v)
{
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    if ( v) // Ensure that models in the other viewers are not controlled.
    {
        activateAllOnViewer( _v0, false);
        activateAllOnViewer( _v2, false);
    }   // end if
    checkEnableLeftToCentre();
    checkEnableRightToCentre();
    emit onSelected( fcont, v);
}   // end doOnCentreViewerSetSelected


// private slot
void MultiFaceModelViewer::doOnRightViewerSetSelected( FaceControl* fcont, bool v)
{
    checkEnableRightToCentre();
    if ( v) // Ensure that models in the other viewers are not controlled.
    {
        activateAllOnViewer( _v0, false);
        activateAllOnViewer( _v1, false);
    }   // end if
    checkEnableCentreToRight();
    checkEnableCentreToLeft();
    checkEnableLeftToCentre();
    emit onSelected( fcont, v);
}   // end doOnRightViewerSetSelected


// private
void MultiFaceModelViewer::checkEnableLeftToCentre()
{
    _moveLeftToCentreAction->setEnabled( !_v0->selected().empty());
    _copyLeftToCentreAction->setEnabled( canCopyTo( _v0, _v1));
}   // end checkEnableLeftToCentre


// private
void MultiFaceModelViewer::checkEnableCentreToLeft()
{
    _moveCentreToLeftAction->setEnabled( !_v1->selected().empty());
    _copyCentreToLeftAction->setEnabled( canCopyTo( _v1, _v0));
}   // end checkEnableCentreToLeft


// private
void MultiFaceModelViewer::checkEnableCentreToRight()
{
    _moveCentreToRightAction->setEnabled( !_v1->selected().empty());
    _copyCentreToRightAction->setEnabled( canCopyTo( _v1, _v2));
}   // end checkEnableCentreToRight


// private
void MultiFaceModelViewer::checkEnableRightToCentre()
{
    _moveRightToCentreAction->setEnabled( !_v2->selected().empty());
    _copyRightToCentreAction->setEnabled( canCopyTo( _v2, _v1));
}   // end checkEnableRightToCentre


// private
// Find the set of models than can go (move or copy) to tv from sv
size_t MultiFaceModelViewer::canCopyTo( FaceModelViewer* sv, FaceModelViewer* tv, FaceControlSet* totv) const
{
    const FaceControlSet& fcs = sv->selected();
    size_t totvCount = 0;

    if ( totv)
    {
        for ( FaceControl* fc : fcs)
        {
            if ( !tv->isAttached(fc))
            {
                totv->insert(fc);
                totvCount++;
            }   // end if
        }   // end foreach
    }   // end if
    else
    {
        for ( FaceControl* fc : fcs)
        {
            if ( !tv->isAttached(fc))
                totvCount++;
        }   // end foreach
    }   // end else

    return totvCount;
}   // end canCopyTo
