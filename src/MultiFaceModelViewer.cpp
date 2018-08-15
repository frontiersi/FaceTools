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
#include <FaceControl.h>
#include <QVBoxLayout>
using FaceTools::MultiFaceModelViewer;
using FaceTools::FaceModelViewer;

namespace {
QToolButton* makeButton()
{
    QToolButton* b = new QToolButton;
    b->setMinimumSize( QSize(27,27));
    b->setMaximumSize( QSize(27,27));
    b->setIconSize( QSize(27,27));
    b->setStyleSheet( "QToolButton::menu-indicator { image: none; }");
    return b;
}   // makeButton

}   // end namespace


// private
void MultiFaceModelViewer::addCommonButtons( QLayout* l)
{
    QToolButton* ss = makeButton();
    l->addWidget(ss);
    _saveImageButtons.push_back(ss);
    QToolButton* cb = makeButton();
    l->addWidget(cb);
    _resetCameraButtons.push_back( cb);
}   // end addCommonButtons


// public
MultiFaceModelViewer::MultiFaceModelViewer( QWidget *parent) : QWidget(parent)
{
    _v0 = new FaceModelViewer( this); // Left
    _v1 = new FaceModelViewer( this); // Middle
    _v2 = new FaceModelViewer( this); // Right

    connect( _v0, &FaceModelViewer::onAttached, [this](){ setViewerVisible( 0, !_v0->attached().empty());});
    connect( _v0, &FaceModelViewer::onDetached, [this](){ setViewerVisible( 0, !_v0->attached().empty());});
    connect( _v2, &FaceModelViewer::onAttached, [this](){ setViewerVisible( 2, !_v2->attached().empty());});
    connect( _v2, &FaceModelViewer::onDetached, [this](){ setViewerVisible( 2, !_v2->attached().empty());});

    QVBoxLayout* v0layout = new QVBoxLayout;
    v0layout->setContentsMargins(0,0,0,0);
    QVBoxLayout* v1layout = new QVBoxLayout;
    v1layout->setContentsMargins(0,0,0,0);
    QVBoxLayout* v2layout = new QVBoxLayout;
    v2layout->setContentsMargins(0,0,0,0);

    _copyButton.resize(4);
    _moveButton.resize(4);
    for ( int i = 0; i < 4; ++i)
    {
        _copyButton[i] = makeButton();
        _moveButton[i] = makeButton();
    }   // end for

    // Left panel
    v0layout->addWidget(_v0);
    QHBoxLayout* h0blayout = new QHBoxLayout;
    h0blayout->setContentsMargins(0,0,0,0);
    h0blayout->addStretch();
    addCommonButtons( h0blayout);
    h0blayout->addWidget( _copyButton[0]);
    h0blayout->addWidget( _moveButton[0]);
    h0blayout->addStretch();
    v0layout->addLayout(h0blayout);

    // Centre panel
    v1layout->addWidget(_v1);
    QHBoxLayout* h1blayout = new QHBoxLayout;
    h1blayout->setContentsMargins(0,0,0,0);
    h1blayout->addStretch();
    h1blayout->addWidget( _moveButton[1]);
    h1blayout->addWidget( _copyButton[1]);
    addCommonButtons( h1blayout);
    h1blayout->addWidget( _copyButton[2]);
    h1blayout->addWidget( _moveButton[2]);
    h1blayout->addStretch();
    v1layout->addLayout( h1blayout);

    // Right panel
    v2layout->addWidget(_v2);
    QHBoxLayout* h2blayout = new QHBoxLayout;
    h2blayout->setContentsMargins(0,0,0,0);
    h2blayout->addStretch();
    h2blayout->addWidget( _moveButton[3]);
    h2blayout->addWidget( _copyButton[3]);
    addCommonButtons( h2blayout);
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

    _splitter->setCollapsible(0, false);
    _splitter->setCollapsible(1, false);
    _splitter->setCollapsible(2, false);

    setViewerVisible( 0, false);
    setViewerVisible( 2, false);
}   // end ctor


// public
MultiFaceModelViewer::~MultiFaceModelViewer()
{
    _v0->disconnect(this);
    _v1->disconnect(this);
    _v2->disconnect(this);
}   // end dtor


// Left viewer
void MultiFaceModelViewer::setCopyLeftToCentreAction( QAction *action) { _copyButton[0]->setDefaultAction(action);}
void MultiFaceModelViewer::setMoveLeftToCentreAction( QAction *action) { _moveButton[0]->setDefaultAction(action);}
void MultiFaceModelViewer::setLeftResetCameraAction( QAction *a) { _resetCameraButtons[0]->setDefaultAction(a);}
void MultiFaceModelViewer::setLeftSaveImageAction( QAction *a) { _saveImageButtons[0]->setDefaultAction(a);}

// Centre viewer
void MultiFaceModelViewer::setMoveCentreToLeftAction( QAction *action) { _moveButton[1]->setDefaultAction(action);}
void MultiFaceModelViewer::setCopyCentreToLeftAction( QAction *action) { _copyButton[1]->setDefaultAction(action);}
void MultiFaceModelViewer::setCopyCentreToRightAction( QAction *action) { _copyButton[2]->setDefaultAction(action);}
void MultiFaceModelViewer::setMoveCentreToRightAction( QAction *action) { _moveButton[2]->setDefaultAction(action);}
void MultiFaceModelViewer::setCentreResetCameraAction( QAction *a) { _resetCameraButtons[1]->setDefaultAction(a);}
void MultiFaceModelViewer::setCentreSaveImageAction( QAction *a) { _saveImageButtons[1]->setDefaultAction(a);}

// Right viewer
void MultiFaceModelViewer::setMoveRightToCentreAction( QAction *action) { _moveButton[3]->setDefaultAction(action);}
void MultiFaceModelViewer::setCopyRightToCentreAction( QAction *action) { _copyButton[3]->setDefaultAction(action);}
void MultiFaceModelViewer::setRightResetCameraAction( QAction *a) { _resetCameraButtons[2]->setDefaultAction(a);}
void MultiFaceModelViewer::setRightSaveImageAction( QAction *a) { _saveImageButtons[2]->setDefaultAction(a);}


// public slot
void MultiFaceModelViewer::setViewerVisible( int idx, bool visible)
{
    assert(idx == 0 || idx == 2);
    setUpdatesEnabled(false);   // Pause widget update to lessen appearance of flicker
    QList<int> widths = _splitter->sizes();
    int sum = widths[0] + widths[1] + widths[2];

    if ( visible && widths[idx] == 0)
        widths[idx] = widths[1];
    else if ( !visible && widths[idx] > 0)
        widths[idx] = 0;
    _splitter->setSizes( widths);
    _splitter->widget( idx)->setVisible(visible);

    setUpdatesEnabled(true);
}   // end setViewerVisible
