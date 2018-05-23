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
using FaceTools::Action::ActionSelect;
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
    b->setMinimumSize( QSize(22,22));
    b->setMaximumSize( QSize(22,22));
    b->setIconSize( QSize(22,22));
    b->setStyleSheet( "QToolButton::menu-indicator { image: none; }");
    return b;
}   // makeButton


// public
MultiFaceModelViewer::MultiFaceModelViewer( ActionSelect* s, QWidget *parent)
    : QWidget(parent), _selector(s)
{
    _v0 = new FaceModelViewer( this); // Left
    _v1 = new FaceModelViewer( this); // Middle
    _v2 = new FaceModelViewer( this); // Right

    connect( _selector, &ActionSelect::onSelect, this, &MultiFaceModelViewer::doOnSelect);
    _selector->setViewer(_v1);

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
    h0blayout->addStretch();
    addCommonButtons( h0blayout, _v0);
    h0blayout->addWidget( makeButton( _copyLeftToCentreAction));
    h0blayout->addWidget( makeButton( _moveLeftToCentreAction));
    h0blayout->addStretch();
    v0layout->addLayout(h0blayout);

    // Centre panel
    v1layout->addWidget(_v1);
    QHBoxLayout* h1blayout = new QHBoxLayout;
    h1blayout->setContentsMargins(0,0,0,0);
    h1blayout->addStretch();
    h1blayout->addWidget( makeButton( _moveCentreToLeftAction));
    h1blayout->addWidget( makeButton( _copyCentreToLeftAction));
    addCommonButtons( h1blayout, _v1);
    h1blayout->addWidget( makeButton( _copyCentreToRightAction));
    h1blayout->addWidget( makeButton( _moveCentreToRightAction));
    h1blayout->addStretch();
    v1layout->addLayout( h1blayout);

    // Right panel
    v2layout->addWidget(_v2);
    QHBoxLayout* h2blayout = new QHBoxLayout;
    h2blayout->setContentsMargins(0,0,0,0);
    h2blayout->addStretch();
    h2blayout->addWidget( makeButton( _moveRightToCentreAction));
    h2blayout->addWidget( makeButton( _copyRightToCentreAction));
    addCommonButtons( h2blayout, _v2);
    h2blayout->addStretch();
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
    FaceModelSet fms = _selector->available().models();
    std::for_each( std::begin(fms), std::end(fms), [this](auto fm){this->remove(fm);});
}   // end dtor


// private slot
void MultiFaceModelViewer::doOnSelect( FaceControl* fc, bool v)
{
    checkEnableLeftToCentre();
    checkEnableCentreToLeft();
    checkEnableCentreToRight();
    checkEnableRightToCentre();
}   // end doOnSelect


// private slot
void MultiFaceModelViewer::doOnViewerEntered()
{
    ModelViewerEntryExitInteractor* vi = qobject_cast<ModelViewerEntryExitInteractor*>( sender());
    ModelViewer* tv = vi->viewer();      // Target viewer (newly active)
    ModelViewer* sv = _selector->viewer();
    const size_t mcount = sv->transferInteractors( tv); // Transfer interactors to target viewer

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
    _selector->removeFaceControl( fc);
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
    FaceControlSet fcs = _selector->selected();  // Copy out
    for ( FaceControl* fc : fcs)
    {
        FaceControl* tfc = tv->get(fc->data()); // Any FaceControl instance on the target viewer.

        // If the target doesn't yet have the data, move it to the target viewer.
        if ( tfc == NULL)
        {
            vwrs.insert(fc->viewer());  // Collect viewer for update render
            _selector->select( fc, false);
            fc->viewer()->detach(fc);   // Detach from source viewer
            tv->attach( fc);            // Attach to target viewer
            _selector->select( fc, true);
        }   // end if
        else
        {   // Otherwise, the model is duplicated on source and target, so delete the FaceControl on the
            // source viewer, and make the FaceControl on the target viewer the newly selected.
            deleteFaceControl( fc);
            _selector->select( tfc, true);
        }   // end else
    }   // end foreach
    std::for_each(std::begin(vwrs), std::end(vwrs), [](auto v){v->updateRender();});
    tv->updateRender();
}   // end moveViews


// private
void MultiFaceModelViewer::copyViews( FaceModelViewer* tv)
{
    FaceControlSet fcs = _selector->selected();  // Copy out
    for ( FaceControl* fc : fcs)
    {
        FaceControl* fc2 = new FaceControl(fc->data());   // Create copy with same data
        tv->attach( fc2);
        _selector->addFaceControl( fc2);
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
void MultiFaceModelViewer::checkEnableLeftToCentre()
{
    bool m = canMoveFrom(_v0);
    _moveLeftToCentreAction->setEnabled( m);
    _copyLeftToCentreAction->setEnabled( m && canCopyTo( _v1));
}   // end checkEnableLeftToCentre


// private
void MultiFaceModelViewer::checkEnableCentreToLeft()
{
    bool m = canMoveFrom(_v1);
    _moveCentreToLeftAction->setEnabled( m);
    _copyCentreToLeftAction->setEnabled( m && canCopyTo( _v0));
}   // end checkEnableCentreToLeft


// private
void MultiFaceModelViewer::checkEnableCentreToRight()
{
    bool m = canMoveFrom(_v1);
    _moveCentreToRightAction->setEnabled( m);
    _copyCentreToRightAction->setEnabled( m && canCopyTo( _v2));
}   // end checkEnableCentreToRight


// private
void MultiFaceModelViewer::checkEnableRightToCentre()
{
    bool m = canMoveFrom(_v2);
    _moveRightToCentreAction->setEnabled( m);
    _copyRightToCentreAction->setEnabled( m && canCopyTo( _v1));
}   // end checkEnableRightToCentre


// private
bool MultiFaceModelViewer::canCopyTo( FaceModelViewer* tv) const
{
    for ( FaceControl* fc : _selector->selected())
    {
        if ( fc->viewer() != tv && tv->get(fc->data()) == NULL)
            return true;
    }   // end foreach
    return false;
}   // end canCopyTo


// private
bool MultiFaceModelViewer::canMoveFrom( FaceModelViewer* sv) const
{
    for ( FaceControl* fc : _selector->selected())
    {
        if ( fc->viewer() == sv)
            return true;
    }   // end foreach
    return false;
}   // end canMoveFrom
