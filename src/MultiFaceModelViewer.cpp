/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
#include <FaceModelManager.h>
#include <ModelSelector.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
using FaceTools::MultiFaceModelViewer;
using FaceTools::FaceModelViewer;
using FaceTools::Vis::FV;
using FaceTools::FM;

namespace {
QToolButton* makeButton()
{
    QToolButton* b = new QToolButton;
    b->setMinimumSize( QSize(27,27));
    b->setMaximumSize( QSize(27,27));
    b->setIconSize( QSize(27,27));
    //b->setStyleSheet( "QToolButton::menu-indicator { image: none; }");
    b->setStyleSheet( "\
        QToolButton{ \
            border: none; \
        } \
        QToolButton:hover{ \
            border: 1px solid #8f8f91; \
            border-radius: 4px; \
        }");

    return b;
}   // makeButton

}   // end namespace


// public
MultiFaceModelViewer::MultiFaceModelViewer( QWidget *parent) : QWidget(parent)
{
    for ( size_t i = 0; i < 3; ++i) // Left (0), middle (1), and right (2) viewers
    {
        _fmvs.push_back( new FMV(this));
        connect( _fmvs[i], &FMV::onAttached, [=]( FV* fv){ doOnViewerChanged( i, fv);});
        connect( _fmvs[i], &FMV::onDetached, [=]( FV* fv){ doOnViewerChanged( i, fv);});
        _modelLists.push_back( new QComboBox(this));
        connect( _modelLists[i], QOverload<const QString&>::of( &QComboBox::activated),
                 [=](const QString &txt){ doOnComboBoxChanged( i, txt);});
    }   // end for

    QVBoxLayout* v0layout = new QVBoxLayout;
    v0layout->setContentsMargins(0,0,0,0);
    QVBoxLayout* v1layout = new QVBoxLayout;
    v1layout->setContentsMargins(0,0,0,0);
    QVBoxLayout* v2layout = new QVBoxLayout;
    v2layout->setContentsMargins(0,0,0,0);

    v0layout->addWidget( _fmvs[0]);
    v1layout->addWidget( _fmvs[1]);
    v2layout->addWidget( _fmvs[2]);

    _copyButton.resize(4);
    _moveButton.resize(4);
    for ( size_t i = 0; i < 4; ++i)
    {
        _copyButton[i] = makeButton();
        _moveButton[i] = makeButton();
    }   // end for

    // Left panel
    QHBoxLayout* hl0 = new QHBoxLayout;
    hl0->setContentsMargins(0,0,0,0);
    hl0->addStretch(1);
    hl0->addWidget( _modelLists[0],7);
    hl0->addWidget( _copyButton[0],1);
    hl0->addWidget( _moveButton[0],1);
    v0layout->addLayout( hl0);

    // Centre panel
    QHBoxLayout* hl1 = new QHBoxLayout;
    hl1->setContentsMargins(0,0,0,0);
    hl1->addWidget( _moveButton[1],1);
    hl1->addWidget( _copyButton[1],1);
    hl1->addWidget( _modelLists[1],6);
    hl1->addWidget( _copyButton[2],1);
    hl1->addWidget( _moveButton[2],1);
    v1layout->addLayout( hl1);

    // Right panel
    QHBoxLayout* hl2 = new QHBoxLayout;
    hl2->setContentsMargins(0,0,0,0);
    hl2->addWidget( _moveButton[3],1);
    hl2->addWidget( _copyButton[3],1);
    hl2->addWidget( _modelLists[2],7);
    hl2->addStretch(1);
    v2layout->addLayout( hl2);

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


// Left viewer
void MultiFaceModelViewer::setCopyLeftToCentreAction( QAction *action) { _copyButton[0]->setDefaultAction(action);}
void MultiFaceModelViewer::setMoveLeftToCentreAction( QAction *action) { _moveButton[0]->setDefaultAction(action);}

// Centre viewer
void MultiFaceModelViewer::setMoveCentreToLeftAction( QAction *action) { _moveButton[1]->setDefaultAction(action);}
void MultiFaceModelViewer::setCopyCentreToLeftAction( QAction *action) { _copyButton[1]->setDefaultAction(action);}
void MultiFaceModelViewer::setCopyCentreToRightAction( QAction *action) { _copyButton[2]->setDefaultAction(action);}
void MultiFaceModelViewer::setMoveCentreToRightAction( QAction *action) { _moveButton[2]->setDefaultAction(action);}

// Right viewer
void MultiFaceModelViewer::setMoveRightToCentreAction( QAction *action) { _moveButton[3]->setDefaultAction(action);}
void MultiFaceModelViewer::setCopyRightToCentreAction( QAction *action) { _copyButton[3]->setDefaultAction(action);}


// private
void MultiFaceModelViewer::setViewerVisible( size_t idx, bool visible)
{
    assert(idx == 0 || idx == 2);
    setUpdatesEnabled(false);   // Pause widget update to lessen appearance of flicker
    QList<int> widths = _splitter->sizes();
    const int i = int(idx);

    if ( visible && widths[i] == 0)
        widths[i] = widths[1];
    else if ( !visible && widths[i] > 0)
        widths[i] = 0;
    _splitter->setSizes( widths);
    _splitter->widget( i)->setVisible(visible);

    setUpdatesEnabled(true);
}   // end setViewerVisible


namespace  {

QString currentModelName( const FM* fm)
{
    QString cname = FaceTools::FileIO::FMM::filepath(fm).c_str();
    if ( !fm->isSaved())
        cname += " (*)";
    return cname;
}   // end currentModelName

}   // end namespace


// public slot
void MultiFaceModelViewer::doOnUpdateModelLists( const FM* fm)
{
    if ( !fm)
        return;
    QString pname;
    if ( _attachedNames.count(fm) > 0)
        pname = _attachedNames.at(fm);  // Previous name
    const QString cname = currentModelName(fm);   // Current (possibly different) name

    bool isAttached = false;
    const size_t n = _fmvs.size();
    for ( size_t i = 0; i < n; ++i)
    {
        const FMV* fmv = _fmvs.at(i);
        QComboBox* cbox = _modelLists.at(i);
        const int listIndex = cbox->findText(pname);    // Will be -1 if adding for first time
        cbox->removeItem( listIndex);   // remove does nothing if listIndex == -1

        if ( fmv->isAttached(fm))   // Attached to viewer fmv so update the name in the dropdown
        {
            cbox->insertItem( listIndex, cname);    // Adds at top if listIndex == -1
            cbox->setCurrentIndex( listIndex);
            isAttached = true;
        }   // end if
    }   // end for

    _attachedNames.erase(fm);
    if ( isAttached)
        _attachedNames[fm] = cname; // Update the name for the attached model
}   // end doOnUpdateModelLists


// private
void MultiFaceModelViewer::doOnComboBoxChanged( size_t i, const QString &txt)
{
    const std::string fname = txt.split( " (*)")[0].toStdString(); // Strip "(*)" if present
    FM* fm = FaceTools::FileIO::FMM::model( fname);
    assert(fm);
    FV* fv = _fmvs[i]->get(fm);
    assert(fv);
    FaceTools::Action::ModelSelector::setSelected(fv);
}   // end doOnComboBoxChanged


// private
void MultiFaceModelViewer::doOnViewerChanged( size_t idx, const FV *fv)
{
    doOnUpdateModelLists( fv->data());
    if ( idx != 1)
        setViewerVisible( idx, !_fmvs.at(idx)->attached().empty());
}   // end doOnViewerChanged
