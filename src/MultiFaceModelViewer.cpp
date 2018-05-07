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
using FaceTools::Interactor::ModelViewerEntryExitInteractor;
using FaceTools::Interactor::ModelSelectInteractor;
using FaceTools::MultiFaceModelViewer;
using FaceTools::FaceModelViewer;
using FaceTools::ModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceModelSet;
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
MultiFaceModelViewer::MultiFaceModelViewer( QWidget *parent)
    : QWidget(parent), _selector(true)  /* exclusive select */
{
    _v0 = new FaceModelViewer( this); // Left
    _v1 = new FaceModelViewer( this); // Middle
    _v2 = new FaceModelViewer( this); // Right
    _selector.setViewer(_v1);   // Centre viewer is initial

    _i0 = new ModelViewerEntryExitInteractor(_v0);
    _i1 = new ModelViewerEntryExitInteractor(_v1);
    _i2 = new ModelViewerEntryExitInteractor(_v2);
    connect( _i0, &ModelViewerEntryExitInteractor::onEnter, this, &MultiFaceModelViewer::doOnViewerEntered);
    connect( _i1, &ModelViewerEntryExitInteractor::onEnter, this, &MultiFaceModelViewer::doOnViewerEntered);
    connect( _i2, &ModelViewerEntryExitInteractor::onEnter, this, &MultiFaceModelViewer::doOnViewerEntered);

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
    _moveLeftToCentreAction = new QAction( goRightIcon, "Move Right", this);
    connect( _moveLeftToCentreAction, &QAction::triggered, this, &MultiFaceModelViewer::moveLeftToCentre);
    _copyLeftToCentreAction = new QAction( copyRightIcon, "Copy Right", this);
    connect( _copyLeftToCentreAction, &QAction::triggered, this, &MultiFaceModelViewer::copyLeftToCentre);

    // Centre panel actions
    _moveCentreToRightAction = new QAction( goRightIcon, "Move Right", this);
    connect( _moveCentreToRightAction, &QAction::triggered, this, &MultiFaceModelViewer::moveCentreToRight);
    _copyCentreToRightAction = new QAction( copyRightIcon, "Copy Right", this);
    connect( _copyCentreToRightAction, &QAction::triggered, this, &MultiFaceModelViewer::copyCentreToRight);
    _moveCentreToLeftAction = new QAction( goLeftIcon, "Move Left", this);
    connect( _moveCentreToLeftAction, &QAction::triggered, this, &MultiFaceModelViewer::moveCentreToLeft);
    _copyCentreToLeftAction = new QAction( copyLeftIcon, "Copy Left", this);
    connect( _copyCentreToLeftAction, &QAction::triggered, this, &MultiFaceModelViewer::copyCentreToLeft);

    // Right panel actions
    _moveRightToCentreAction = new QAction( goLeftIcon, "Move Left", this);
    connect( _moveRightToCentreAction, &QAction::triggered, this, &MultiFaceModelViewer::moveRightToCentre);
    _copyRightToCentreAction = new QAction( copyLeftIcon, "Copy Left", this);
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

    connect( &_selector, &ModelSelectInteractor::onSelected, this, &MultiFaceModelViewer::doOnSelected);
    connect( &_selector, &ModelSelectInteractor::onRemoving, this, &MultiFaceModelViewer::onRemoving);

    _splitter->setCollapsible(1,false);
    setLeftViewerVisible(false);
    setRightViewerVisible(false);
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
}   // end ctor


// public
MultiFaceModelViewer::~MultiFaceModelViewer()
{
    delete _i0;
    delete _i1;
    delete _i2;
    _v0->disconnect(this);
    _v1->disconnect(this);
    _v2->disconnect(this);
    FaceModelSet fms = _selector.available().models();
    std::for_each( std::begin(fms), std::end(fms), [this](auto fm){this->remove(fm);});
}   // end dtor


// public
void MultiFaceModelViewer::addInteractor( FaceTools::Interactor::MVI* mvi)
{
    mvi->setViewer( _selector.viewer());
}   // end addInteractor


// private slot
void MultiFaceModelViewer::doOnSelected( FaceControl* fc, bool v)
{
    assert( fc);
    assert( fc->viewer());
    activateViewer( fc->viewer());
    emit onSelected( fc, v);
}   // end doOnSelected


// private slot
void MultiFaceModelViewer::doOnViewerEntered()
{
    ModelViewerEntryExitInteractor* vi = qobject_cast<ModelViewerEntryExitInteractor*>( sender());
    assert(vi);
    ModelViewer* tv = vi->viewer();      // Target viewer (newly active)
    assert(tv);
    ModelViewer* sv = _selector.viewer();// Source viewer (previously active)
    assert(sv);
    const size_t mcount = sv->transferInteractors( tv); // Transfer interactors to target viewer

    assert(tv == _selector.viewer());

    if ( mcount > 0) // Transfer will have moved the fixed interactors that fired this view, so move them back.
    {
        _i0->setViewer(_v0);
        _i1->setViewer(_v1);
        _i2->setViewer(_v2);
    }   // end if
}   // end doOnViewerEntered


// private
bool MultiFaceModelViewer::deleteFaceControl( FaceControl* fc)
{
    if ( !fc)
        return false;

    FaceModelViewer* v = fc->viewer();
    _selector.remove( fc);  // Causes onSelected(false) followed by onRemoving to fire
    v->detach( fc);         // Sets viewer NULL in the FaceControl
    delete fc;
    v->updateRender();
    if ( v == _v0)
        setLeftViewerVisible( !_v0->attached().empty());
    else if ( v == _v2)
        setRightViewerVisible( !_v2->attached().empty());
    return true;
}   // end deleteFaceControl


// public slot
void MultiFaceModelViewer::remove( FaceModel* fm)
{
    deleteFaceControl( _v0->get(fm));
    deleteFaceControl( _v1->get(fm));
    deleteFaceControl( _v2->get(fm));
}   // end remove


// public slot
void MultiFaceModelViewer::insert( FaceModel* fm)
{
    FaceControl* fc = new FaceControl(fm);
    _v1->attach(fc);
    _selector.add(fc);
}   // end insert


// public slot
void MultiFaceModelViewer::setLeftViewerVisible(bool visible)
{
    QList<int> widths = _splitter->sizes();
    int sum = widths[0] + widths[1] + widths[2];
    if ( visible && widths[0] == 0)
        widths[0] = widths[1];
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
        widths[2] = widths[1];
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
void MultiFaceModelViewer::moveViews( FaceModelViewer* tv)
{
    std::unordered_set<ModelViewer*> vwrs;
    FaceControlSet fcs = _selector.selected();  // Copy out
    for ( FaceControl* fc : fcs)
    {
        // If the target doesn't yet have the model, move it to the target viewer.
        if ( tv->get(fc->data()) == NULL)
        {
            vwrs.insert(fc->viewer());  // Collect viewer for update render
            _selector.setSelected(fc,false);
            fc->viewer()->detach(fc);   // Detach from source viewer
            tv->attach( fc);            // Attach to target viewer
            _selector.setSelected(fc,true);
        }   // end if
        else    // Otherwise, this is a duplicate model on the source viewer, so delete it.
            deleteFaceControl( fc);
    }   // end foreach
    std::for_each(std::begin(vwrs), std::end(vwrs), [](auto v){v->updateRender();});
    tv->updateRender();
}   // end moveViews


// private
void MultiFaceModelViewer::copyViews( FaceModelViewer* tv)
{
    FaceControlSet fcs = _selector.selected();  // Copy out
    for ( FaceControl* fc : fcs)
    {
        FaceControl* fc2 = new FaceControl(fc->data());   // Create copy with same data
        tv->attach( fc2);
        _selector.add( fc2); // Causes onSelected to fire
    }   // end foreach
    tv->updateRender();
}   // end copyViews


// private slot
void MultiFaceModelViewer::moveLeftToCentre()
{
    moveViews( _v1);
    checkEnableLeftToCentre();
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    setLeftViewerVisible( !_v0->attached().empty());
}   // end moveLeftToCentre


// private slot
void MultiFaceModelViewer::copyLeftToCentre()
{
    copyViews( _v1);
    checkEnableLeftToCentre();
}   // end copyLeftToCentre


// private slot
void MultiFaceModelViewer::moveCentreToLeft()
{
    moveViews( _v0);
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    checkEnableLeftToCentre();
    setLeftViewerVisible(true);
}   // end moveCentreToLeft


// private slot
void MultiFaceModelViewer::copyCentreToLeft()
{
    copyViews( _v0);
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    setLeftViewerVisible(true);
}   // end copyCentreToLeft


// private slot
void MultiFaceModelViewer::moveCentreToRight()
{
    moveViews( _v2);
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    checkEnableRightToCentre();
    setRightViewerVisible(true);
}   // end moveCentreToRight


// private slot
void MultiFaceModelViewer::copyCentreToRight()
{
    copyViews( _v2);
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    setRightViewerVisible(true);
}   // end copyCentreToRight


// private slot
void MultiFaceModelViewer::moveRightToCentre()
{
    moveViews( _v1);
    checkEnableRightToCentre();
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    setRightViewerVisible( !_v2->attached().empty());
}   // moveRightToCentre


// private slot
void MultiFaceModelViewer::copyRightToCentre()
{
    copyViews( _v1);
    checkEnableRightToCentre();
}   // copyRightToCentre


// private
void MultiFaceModelViewer::activateViewer( FaceModelViewer* v)
{
    assert(v);
    if ( v == _v0)
        activateLeftViewer();
    else if ( v == _v1)
        activateCentreViewer();
    else
    {
        assert( v == _v2);
        activateRightViewer();
    }   // end else
}   // end activateViewer


// private
void MultiFaceModelViewer::activateLeftViewer()
{
    checkEnableLeftToCentre();
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    checkEnableRightToCentre();
}   // end activateLeftViewer


// private
void MultiFaceModelViewer::activateCentreViewer()
{
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    checkEnableLeftToCentre();
    checkEnableRightToCentre();
}   // end activateCentreViewer


// private
void MultiFaceModelViewer::activateRightViewer()
{
    checkEnableRightToCentre();
    checkEnableCentreToRight();
    checkEnableCentreToLeft();
    checkEnableLeftToCentre();
}   // end activateRightViewer


// private
void MultiFaceModelViewer::checkEnableLeftToCentre()
{
    _moveLeftToCentreAction->setEnabled( canMoveFrom(_v0));
    _copyLeftToCentreAction->setEnabled( canCopyTo(_v1));
}   // end checkEnableLeftToCentre


// private
void MultiFaceModelViewer::checkEnableCentreToLeft()
{
    _moveCentreToLeftAction->setEnabled( canMoveFrom(_v1));
    _copyCentreToLeftAction->setEnabled( canCopyTo(_v0));
}   // end checkEnableCentreToLeft


// private
void MultiFaceModelViewer::checkEnableCentreToRight()
{
    _moveCentreToRightAction->setEnabled( canMoveFrom(_v1));
    _copyCentreToRightAction->setEnabled( canCopyTo(_v2));
}   // end checkEnableCentreToRight


// private
void MultiFaceModelViewer::checkEnableRightToCentre()
{
    _moveRightToCentreAction->setEnabled( canMoveFrom(_v2));
    _copyRightToCentreAction->setEnabled( canCopyTo(_v1));
}   // end checkEnableRightToCentre


// private
bool MultiFaceModelViewer::canCopyTo( FaceModelViewer* tv) const
{
    const FaceControlSet& fcs = _selector.selected();
    for ( FaceControl* fc : fcs)
    {
        if ( tv->get(fc->data()) == NULL)
            return true;
    }   // end foreach
    return false;
}   // end canCopyTo


// private
bool MultiFaceModelViewer::canMoveFrom( FaceModelViewer* sv) const
{
    const FaceControlSet& fcs = _selector.selected();
    for ( FaceControl* fc : fcs)
    {
        if ( sv->get(fc->data()))
            return true;
    }   // end foreach
    return false;
}   // end canMoveFrom
