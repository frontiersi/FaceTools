/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
bool LabelsVisualisation<T>::isAvailable( const FV *fv) const
{
    return T().canCreateLabels( fv->data());
}   // end isAvailable


template <class T>
bool LabelsVisualisation<T>::applyToAllInViewer() const
{
    return T().applyToAllInViewer();
}   // end applyToAllInViewer


template <class T>
bool LabelsVisualisation<T>::applyToAllViewers() const
{
    return T().applyToAllViewers();
}   // end applyToAllViewers


template <class T>
void LabelsVisualisation<T>::purge( const FV* fv) { _views.erase(fv);}


template <class T>
void LabelsVisualisation<T>::setVisible( FV* fv, bool v)
{
    if ( _views.count(fv) > 0)
        _views.at(fv).setVisible( v, fv->viewer());
}   // end setVisible


template <class T>
bool LabelsVisualisation<T>::isVisible( const FV* fv) const
{
    return _views.count(fv) > 0 && _views.at(fv).isVisible();
}   // end isVisible


template <class T>
void LabelsVisualisation<T>::refresh( FV* fv)
{
    const QColor bg = fv->viewer()->backgroundColour();
    const FM *fm = fv->data();
    T& lv = _views[fv];
    //const QColor fg = chooseContrasting(bg);
    const QColor fg = Qt::blue;
    lv.setColours( fg, bg);
    lv.refresh( fv->data());
    lv.transform( fm->transformMatrix());
}   // end refresh


template <class T>
void LabelsVisualisation<T>::syncTransform( const FV *fv)
{
    T& lv = _views[fv];
    if (lv.isVisible() || lv.moving())
    {
        const Mat4f dt = fv->data()->transformMatrix();  // Data transform
        const Mat4f vt = r3dvis::toEigen( fv->transformMatrix());

        // If the data transform is different to the view transform we are in the middle
        // of dynamically moving the FaceView so hide the visualisation, otherwise
        // transform the visualisation to match the model's transform.
        if ( dt == vt)
        {
            lv.transform( dt);
            lv.setVisible( true, const_cast<FV*>(fv)->viewer());
        }   // end if
        else
        {
            lv.setVisible( false, const_cast<FV*>(fv)->viewer());
            lv.setMoving( true);
        }   // end else
    }   // end if
}   // end syncTransform
