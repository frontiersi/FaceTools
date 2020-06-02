/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

template <class T>
LabelsVisualisation<T>::~LabelsVisualisation()
{
    while (!_views.empty())
        purge( _views.begin()->first);
}   // end dtor


template <class T>
bool LabelsVisualisation<T>::isAvailable( const FV *fv, const QPoint*) const
{
    return T().canCreateLabels( fv->data());
}   // end isAvailable


template <class T>
void LabelsVisualisation<T>::apply( const FV* fv, const QPoint*)
{
    if ( _views.count(fv) == 0)
        _views[fv].refresh( fv->data());
}   // end apply


template <class T>
void LabelsVisualisation<T>::purge( const FV* fv)
{
    if (_views.count(fv) > 0)
        _views.erase(fv);
}   // end purge


template <class T>
void LabelsVisualisation<T>::setVisible( FV* fv, bool v)
{
    if (_views.count(fv) > 0)
    {
        _views.at(fv).setVisible( v, fv->viewer());
        syncWithViewTransform( fv);
    }   // end if
}   // end setVisible


template <class T>
bool LabelsVisualisation<T>::isVisible( const FV* fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).isVisible();
}   // end isVisible


template <class T>
void LabelsVisualisation<T>::syncWithViewTransform( const FV *fv)
{
    if ( _views.count(fv) > 0 && (_views.at(fv).isVisible() || _views.at(fv).moving()))
    {
        const Mat4f& dt = fv->data()->transformMatrix();  // Data transform
        const Mat4f vt = r3dvis::toEigen( fv->transformMatrix());

        // If the data transform is different to the view transform we are in the middle
        // of dynamically moving the FaceView so hide the visualisation, otherwise
        // transform the visualisation to match the model's transform.
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
}   // end syncWithViewTransform


template <class T>
void LabelsVisualisation<T>::refreshState( const FV *fv)
{
    const QColor bg = fv->viewer()->backgroundColour();
    T& lv = _views.at(fv);
    lv.setColours( chooseContrasting(bg), bg);
    lv.refresh( fv->data());
}   // end refreshState
