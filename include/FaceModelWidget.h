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

#ifndef FACE_TOOLS_FACE_MODEL_WIDGET_H
#define FACE_TOOLS_FACE_MODEL_WIDGET_H

#include "InteractiveModelViewer.h"
#include "FaceViewComboBox.h"

namespace FaceTools
{

class FaceTools_EXPORT FaceModelWidget : public QWidget
{ Q_OBJECT
public:
    FaceModelWidget( InteractiveModelViewer*, const QList<QAction*>*, QWidget *parent=NULL);
    virtual ~FaceModelWidget();

    // Get the models from this widget into the given set returning the number
    // of models added. If the widget contains multiple views of the same model,
    // they are ignored (no model duplicates are allowed).
    size_t getModels( boost::unordered_set<FaceModel*>&) const;
    size_t getNumModels() const; // Get the number of models displayed in this widget.
    size_t getNumViews() const;  // Get the number of views displayed in this widget (possibly > getNumModels())

    const FaceView* getActiveView() const;

public slots:
    const std::string& addView( FaceModel*);  // Also sets the view as active and returns the view name.
    size_t removeModel( FaceModel*);          // Remove all views of the given model returning the number of views removed.
    void removeView( const std::string&);     // Remove single view

signals:
    void onViewSelected( const std::string&);   // Signal the newly active view title

private:
    InteractiveModelViewer* _viewer;
    const QList<QAction*> *_xactions;
    FaceViewComboBox* _combo;

    FaceModelWidget( const FaceModelWidget&); // No copy
    void operator=( const FaceModelWidget&);  // No copy
};  // end class

}   // end namespace

#endif

