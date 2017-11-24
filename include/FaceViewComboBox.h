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

#ifndef FACE_TOOLS_FACE_VIEW_COMBO_BOX_H
#define FACE_TOOLS_FACE_VIEW_COMBO_BOX_H

#include "ModelInteractor.h"
#include <QComboBox>

namespace FaceTools
{

typedef ModelInteractor Mint;

class FaceTools_EXPORT FaceViewComboBox : public QComboBox
{ Q_OBJECT
public:
    explicit FaceViewComboBox( QWidget* parent=NULL);

    void addView( Mint*);               // Add a view
    void removeView( Mint*);            // Remove a single view.

signals:
    void onActivated( Mint*);           // When selected from the dropdown

private slots:
    void onSelectedRow( int);
    void changeViewNames( FaceModel*);
    void onEditedViewName( const QString&);

private:
    static int s_mintKey;
    boost::unordered_map<FaceModel*, boost::unordered_set<int> > _viewKeys; // How models map to their view keys
    boost::unordered_map<int, std::string> _vnameLookup;       // keys to names
    boost::unordered_map<int, Mint*> _mintLookup;              // keys to Mint
    boost::unordered_map<Mint*, int> _mintLookupR;             // Mint to keys

    std::string createViewName( FaceModel*) const;
    int getRowFromName( const std::string&, int&) const;
    FaceViewComboBox( const FaceViewComboBox&); // No copy
    void operator=( const FaceViewComboBox&);   // No copy
};  // end class

}   // end namespace

#endif

