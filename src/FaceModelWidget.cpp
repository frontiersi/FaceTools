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

#include <FaceModelWidget.h>
#include <QVBoxLayout>
#include <cassert>
using FaceTools::FaceModel;
using FaceTools::FaceModelWidget;

// public
FaceModelWidget::FaceModelWidget( FaceTools::InteractiveModelViewer* viewer, const QList<QAction*>* xactions, QWidget *parent)
    : QWidget(parent),
      _viewer(viewer),
      _xactions(xactions),
      _viewerFrame( new QFrame),
      _combo( new FaceTools::FaceViewComboBox( viewer, this))
{
    connect( _combo, &FaceTools::FaceViewComboBox::onMadeActive, this, &FaceModelWidget::onMadeActive);
    setLayout( new QVBoxLayout);
    layout()->setContentsMargins(QMargins(2,2,2,2));
    layout()->addWidget( _viewerFrame);
    layout()->addWidget( _combo);
}   // end ctor

// public
FaceModelWidget::~FaceModelWidget()
{
    // NB FaceModelTabWidget should have reparented the viewer layout!
    if ( _viewerFrame->layout() != NULL)
        std::cerr << "[WARNING] FaceTools::FaceModelWidget::~FaceModelWidget: Viewer layout not NULL - deleting!" << std::endl;
}   // end dtor

// public
size_t FaceModelWidget::getModels( boost::unordered_set<FaceModel*>& fmodels) const { return _combo->getModels( fmodels);}
size_t FaceModelWidget::getNumModels() const { return _combo->getNumModels();}
size_t FaceModelWidget::getNumViews() const { return _combo->count();}
FaceTools::Mint* FaceModelWidget::getActive( std::string *vname) const { return _combo->getActive( vname);}
void FaceModelWidget::reparentViewer( QLayout* vlayout) { _viewerFrame->setLayout(vlayout);}


// public slots
const std::string& FaceModelWidget::addView( FaceModel* fmodel) { return _combo->addView( fmodel, _xactions);}
void FaceModelWidget::removeView( const std::string& vname) { _combo->removeView(vname);}
size_t FaceModelWidget::removeModel( FaceModel* fmodel) { return _combo->removeModel(fmodel);}

