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

#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <FaceModelViewer.h>
#include <iomanip>
using FaceTools::FaceModel;
using FaceTools::FaceControl;
using FaceTools::FaceModelViewer;
using FaceTools::Vis::FaceView;


FaceControl::FaceControl( FaceModel* fm, FaceModelViewer* viewer)
    : _fdata(fm), _fview(nullptr)
{
    _fview = new FaceView(this);
    _fdata->lockForWrite();
    _fdata->_fcs.insert(this);
    _fdata->unlock();
    setViewer(viewer);
    _fview->reset();    // Rebuild visualisation models
    viewer->attach(this);
}   // end ctor


FaceControl::~FaceControl()
{
    viewer()->detach(this);
    _fdata->lockForWrite();
    _fdata->_fcs.erase(this);
    _fdata->unlock();
    delete _fview;
}   // end dtor


void FaceControl::setViewer( FaceModelViewer* v) { _fview->setViewer(v);}
FaceModelViewer* FaceControl::viewer() const { return static_cast<FaceModelViewer*>(_fview->viewer());}
