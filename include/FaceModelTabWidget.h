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

#ifndef FACE_TOOLS_FACE_MODEL_TAB_WIDGET_H
#define FACE_TOOLS_FACE_MODEL_TAB_WIDGET_H

#include "FaceModelWidget.h"
#include <QTabWidget>

namespace FaceTools
{

class FaceTools_EXPORT FaceModelTabWidget : public QTabWidget
{ Q_OBJECT
public:
    FaceModelTabWidget( QWidget *parent=NULL);
    virtual ~FaceModelTabWidget();

    // All added QActions must have parents deriving from FaceAction.
    // Returns true if added successfully. If desired that the action
    // is available via the context menu, use addContextActionGroup.
    bool addAction( QAction*);

    // These actions may be accessed directly by the user via context menu.
    // Adding multiple groups places separators between them on the menu.
    bool addContextActionGroup( QActionGroup*);

    // ALL actions must be added before creating tabs (FaceModelWidgets) or
    // the actions will NOT be available to these widgets!

signals:
    void activated( const std::string&);    // Passes view title

public slots:
    int addTabWidget( FaceModel*);   // Add model as new tab returning tab index and making it active.
    size_t removeTabWidget( int);    // Remove the tab and all models in it returning the number of models removed.
    void addViewToTab( int, FaceModel*);         // Add a view to a tab returning tab.
    size_t removeModelFromTab( int, FaceModel*); // Remove model from tab returning num views removed.

    // Replace all the tabs in this tab group with a single tab containing
    // views of all the models in the tab group. Does nothing if no tabs yet.
    void consolidateTabs();

private slots:
    void showContextMenu( const QPoint&);

private:
    InteractiveModelViewer *_viewer; // Viewer shared between models in tab group
    QActionGroup _xactions;
    QMenu _cmenu;

    FaceModelTabWidget( const FaceModelTabWidget&); // No copy
    void operator=( const FaceModelTabWidget&);     // No copy
};  // end class

}   // end namespace

#endif

