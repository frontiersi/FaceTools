/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_MULTI_FACE_MODEL_VIEWER_H
#define FACE_TOOLS_MULTI_FACE_MODEL_VIEWER_H

#include "FaceModelViewer.h"
#include <QToolButton>
#include <QComboBox>
#include <QSplitter>

namespace FaceTools {

class FaceTools_EXPORT MultiFaceModelViewer : public QWidget
{ Q_OBJECT
public:
    explicit MultiFaceModelViewer( QWidget *parent=nullptr);
    ~MultiFaceModelViewer() override {}

    FMV* leftViewer() const { return _fmvs.at(0);}
    FMV* centreViewer() const { return _fmvs.at(1);}
    FMV* rightViewer() const { return _fmvs.at(2);}

    void setCopyLeftToCentreAction( QAction*);
    void setMoveLeftToCentreAction( QAction*);

    void setMoveCentreToLeftAction( QAction*);
    void setCopyCentreToLeftAction( QAction*);
    void setCopyCentreToRightAction( QAction*);
    void setMoveCentreToRightAction( QAction*);

    void setMoveRightToCentreAction( QAction*);
    void setCopyRightToCentreAction( QAction*);

public slots:
    void doOnUpdateModelLists();

private:
    std::vector<QToolButton*> _copyButton;
    std::vector<QToolButton*> _moveButton;
    std::vector<QComboBox*> _modelLists;
    std::vector<FMV*> _fmvs;
    QSplitter *_splitter;

    // The reference names (filenames) shown in the dropdown lists for attached models.
    std::unordered_map<const FM*, QString> _attachedNames;

    void _updateModelLists( const FM*);
    void _setViewerVisible( size_t, bool);
    void _doOnViewerChanged( size_t, const Vis::FV*);
    void _doOnComboBoxChanged( size_t, const QString&);

    MultiFaceModelViewer( const MultiFaceModelViewer&) = delete;
    void operator=( const MultiFaceModelViewer&) = delete;
};  // end class

}   // end namespace

#endif
