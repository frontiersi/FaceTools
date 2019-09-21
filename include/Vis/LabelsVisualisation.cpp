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

using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Action::Event;

namespace {
bool nearIdentity( const cv::Matx44d& d, size_t ndp=8)
{
    const cv::Matx44d I = cv::Matx44d::eye();
    for ( int i = 0; i < 16; ++i)
    {
        if ( (RFeatures::roundndp(d.val[i], ndp) - I.val[i]) != 0.0)
            return false;
    }   // end for
    return true;
}   // end nearIdentity

}   // end namespace


template <class T>
LabelsVisualisation<T>::~LabelsVisualisation()
{
    while (!_views.empty())
        purge( const_cast<FV*>(_views.begin()->first), Event::NONE);
}   // end dtor


template <class T>
void LabelsVisualisation<T>::apply( FV* fv, const QPoint*)
{
    if ( _views.count(fv) == 0)
    {
        _views[fv] = new T;
        _views[fv]->refresh( fv->data()->model());
    }   // end if
}   // end apply


template <class T>
bool LabelsVisualisation<T>::purge( FV* fv, Event)
{
    setVisible(fv, false);
    if (_views.count(fv) > 0)
    {
        delete _views[fv];
        _views.erase(fv);
    }   // end if
    return true;
}   // end purge


template <class T>
void LabelsVisualisation<T>::setVisible( FV* fv, bool v)
{
    if (_views.count(fv) > 0)
        _views.at(fv)->setVisible(v, fv->viewer());
}   // end setVisible


template <class T>
bool LabelsVisualisation<T>::isVisible( const FV* fv) const
{
    return _views.count(fv) > 0 ? _views.at(fv)->visible() : false;
}   // end isVisible


template <class T>
void LabelsVisualisation<T>::syncToViewTransform( const FV *fv, const vtkMatrix4x4* d)
{
    if ( _views.count(fv) == 0)
        return;

    if ( !nearIdentity( RVTK::toCV(d)))
        setVisible( const_cast<FV*>(fv), false);
    else
    {
        _views.at(fv)->refresh( fv->data()->model());
        setVisible( const_cast<FV*>(fv), isVisible(fv));
    }   // end else
}   // end syncToViewTransform


template <class T>
void LabelsVisualisation<T>::checkState( const FV* fv)
{
    QColor bg = fv->viewer()->backgroundColour();
    _views.at(fv)->setColours( chooseContrasting(bg), bg);
}   // end checkState
