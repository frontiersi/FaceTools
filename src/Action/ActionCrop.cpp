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

#include <ActionCrop.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>
#include <FaceModelViewer.h>
#include <ObjModelRegionSelector.h>
#include <ObjModelCopier.h>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionCrop;
using FaceTools::Action::ActionRadialSelect;
using FaceTools::Action::ChangeEventSet;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionCrop::ActionCrop( const QString& dn, const QIcon& ico, ActionRadialSelect *rs, QProgressBar* pb)
    : FaceAction(dn, ico), _rsel(rs)
{
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionCrop::testEnabled( const QPoint*) const
{
    //std::cerr << "ActionCrop::testEnabled " << std::boolalpha << (ready1() != nullptr) << " " << _rsel->isChecked() << std::endl;
    return (ready1() != nullptr) && _rsel->isChecked();
}   // end testEnabled


bool ActionCrop::doAction( FaceControlSet& rset, const QPoint&)
{
    assert(_rsel);
    assert(rset.size() == 1);
    FaceControl* fc = rset.first();

    double rad = _rsel->radius( fc);
    cv::Vec3f v = _rsel->centre( fc);

    FaceModel* fm = fc->data();
    fm->lockForWrite();
    int s = fm->kdtree()->find(v);

    using namespace RFeatures;
    ObjModelRegionSelector::Ptr cropper = ObjModelRegionSelector::create( fm->info()->cmodel(), v, s);
    cropper->setRadius( rad);
    IntSet cfids;
    cropper->getRegionFaces( cfids);

    // Copy the subset of faces into a new model
    if ( !cfids.empty())
    {
        ObjModelCopier copier( fm->info()->cmodel());
        std::for_each( std::begin(cfids), std::end(cfids), [&](int fid){ copier.addTriangle(fid);});
        ObjModel::Ptr nmodel = copier.getCopiedModel();
        ObjModelInfo::Ptr nfo = ObjModelInfo::create(nmodel);
        fm->update( nfo);
    }   // end if

    fm->unlock();
    return true;
}   // end doAction
