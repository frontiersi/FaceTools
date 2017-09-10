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
typedef const ModelInteractor CMint;

class FaceTools_EXPORT FaceViewComboBox : public QComboBox
{ Q_OBJECT
public:
    explicit FaceViewComboBox( InteractiveModelViewer*, QWidget* parent=NULL);
    virtual ~FaceViewComboBox();

    size_t getModels( boost::unordered_set<FaceModel*>&) const;
    const std::string& addView( FaceModel*, const QList<QAction*>*); // Insert new view in the combo box, returning view name.
    void removeView( const std::string&);       // Remove a single view with given name.
    size_t removeModel( FaceModel*);            // Remove all views for the given model, returning the number removed.
    CMint* getSelectedView() const;             // Returns the currently selected view or NULL if none selected.
    std::string getSelectedViewName() const;    // Returns the currently selected view name or "" if none selected.
    size_t getNumModels() const;                // Return number of models (use count() for num views).

signals:
    void onViewSelected( const std::string&);

private slots:
    void onSelectedRow( int);
    void changeViewNames( FaceModel*);
    void onEditedViewName( const QString&);

private:
    static int s_mintKey;
    InteractiveModelViewer *_viewer;
    int _curView;
    boost::unordered_map<FaceModel*, boost::unordered_set<int> > _viewKeys; // How models map to their view keys
    boost::unordered_map<int, std::string> _vnameLookup;       // keys to names
    boost::unordered_map<int, Mint*> _mintLookup;              // keys to Mint

    std::string createViewName( FaceModel*) const;
    int getRowFromName( const std::string&, int&) const;
    FaceViewComboBox( const FaceViewComboBox&); // No copy
    void operator=( const FaceViewComboBox&);   // No copy
};  // end class

}   // end namespace

#endif

