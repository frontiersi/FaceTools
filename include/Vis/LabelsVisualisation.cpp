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
        _views[fv].refresh( fv->data()->model());
}   // end apply


template <class T>
bool LabelsVisualisation<T>::purge( FV* fv, Event)
{
    setVisible(fv, false);
    if (_views.count(fv) > 0)
        _views.erase(fv);
    return true;
}   // end purge


template <class T>
void LabelsVisualisation<T>::setVisible( FV* fv, bool v)
{
    if (_views.count(fv) > 0)
    {
        _views.at(fv).setVisible( v, fv->viewer());
        syncToViewTransform( fv, fv->actor()->GetMatrix());
    }   // end if
}   // end setVisible


template <class T>
bool LabelsVisualisation<T>::isVisible( const FV* fv) const
{
    return _views.count(fv) > 0 ? _views.at(fv).visible() : false;
}   // end isVisible


template <class T>
void LabelsVisualisation<T>::syncToViewTransform( const FV *fv, const vtkMatrix4x4* d)
{
    if ( _views.count(fv) > 0 && (_views.at(fv).visible() || _views.at(fv).moving()))
    {
        const cv::Matx44d& dt = fv->data()->model().transformMatrix();  // Data transform
        const cv::Matx44d vt = RVTK::toCV( d);

        // If the model's data transform is different to the model's view transform, hide the visualisation,
        // otherwise if we were moving transform the visualisation to match the model's transform.
        if ( dt == vt)
        {
            _views.at(fv).transform( dt);
            _views.at(fv).setVisible( true, fv->viewer());
        }   // end if
        else
        {
            _views.at(fv).setVisible( false, fv->viewer());
            _views.at(fv).setMoving( true);
        }   // end else
    }   // end if
}   // end syncToViewTransform


template <class T>
void LabelsVisualisation<T>::checkState( const FV* fv)
{
    const QColor bg = fv->viewer()->backgroundColour();
    _views.at(fv).setColours( chooseContrasting(bg), bg);
}   // end checkState
