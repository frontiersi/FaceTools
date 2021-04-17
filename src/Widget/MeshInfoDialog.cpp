/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
 *
 * Cliniface is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cliniface is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <Widget/MeshInfoDialog.h>
#include <ui_MeshInfoDialog.h>
#include <Action/FaceAction.h>
#include <ModelSelect.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QTools/QImageTools.h>
#include <QPushButton>
#include <QSpinBox>
#include <cassert>
using FaceTools::Widget::MeshInfoDialog;
using FaceTools::Action::FaceAction;
using MS = FaceTools::ModelSelect;


MeshInfoDialog::MeshInfoDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::MeshInfoDialog),
    _dialogRootTitle( parent->windowTitle() + " | Model Properties")
{
    _ui->setupUi(this);
    _ui->spatialGroupBox->setTitle( QString("Spatial Extents (%1)").arg( FM::LENGTH_UNITS));
    setWindowTitle( _dialogRootTitle);
    connect( _ui->manifoldSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                             this, &MeshInfoDialog::doOnManifoldIndexChanged);
    setFixedSize( geometry().width(), geometry().height());
    refresh();
}   // end ctor


MeshInfoDialog::~MeshInfoDialog() { delete _ui;}


int MeshInfoDialog::selectedManifold() const
{
    return _ui->manifoldSpinBox->value();
}   // end selectedManifold


void MeshInfoDialog::setDiscardManifoldAction( FaceAction *act)
{
    _ui->discardManifoldButton->setDefaultAction( act->qaction());
}   // end setDiscardManifoldAction


void MeshInfoDialog::setRemoveManifoldsAction( FaceAction *act)
{
    _ui->removeManifoldsButton->setDefaultAction( act->qaction());
}   // end setRemoveManifoldsAction


void MeshInfoDialog::refresh()
{
    setWindowTitle( _dialogRootTitle);
    _ui->modelPolygonsLabel->clear();
    _ui->modelVerticesLabel->clear();
    _ui->modelBoundariesLabel->clear();

    int nm = 0; // Number of manifolds

    const FM *fm = MS::selectedModel();
    if ( fm)
    {
        fm->lockForRead();
        const r3d::Manifolds& manfs = fm->manifolds();
        const r3d::Mesh& cmodel = fm->mesh();
        nm = int(manfs.count());  // Number of manifolds

        //_ui->textureMappedLabel->setText( cmodel.materialIds().empty() ? "No Texture Mapping!" : "");

        _ui->modelPolygonsLabel->setText( QString("%1").arg(cmodel.numFaces()));
        _ui->modelVerticesLabel->setText( QString("%1").arg(cmodel.numVtxs()));
        size_t nb = 0;  // Count up the total number of boundaries
        for ( int i = 0; i < nm; ++i)
            nb += manfs[i].boundaries().count();
        _ui->modelBoundariesLabel->setText( QString("%1").arg(nb));
        fm->unlock();
    }   // end if

    _ui->manifoldSpinBox->setMinimum( 1);
    _ui->manifoldSpinBox->setMaximum( nm);
    _ui->manifoldSpinBox->setEnabled( nm > 1);
    _ui->manifoldSpinBox->setValue( 1);
    doOnManifoldIndexChanged(1);
}   // end refresh


void MeshInfoDialog::doOnManifoldIndexChanged( int i)
{
    _ui->manifoldPolygonsLabel->clear();
    _ui->manifoldVerticesLabel->clear();
    _ui->manifoldBoundariesLabel->clear();
    _ui->manifoldXLabel->clear();
    _ui->manifoldYLabel->clear();
    _ui->manifoldZLabel->clear();

    const FM *fm = MS::selectedModel();
    if ( !fm)
        return;

    fm->lockForRead();
    const r3d::Manifolds& manfs = fm->manifolds();
    assert( i > 0 && i <= int(manfs.count()));
    const int realIdx = i - 1;

    const r3d::Manifold& manf = manfs[realIdx];
    _ui->manifoldPolygonsLabel->setText( QString("%1").arg(manf.faces().size()));
    _ui->manifoldVerticesLabel->setText( QString("%1").arg(manf.vertices().size()));
    _ui->manifoldBoundariesLabel->setText( QString("%1").arg(manf.boundaries().count()));

    const r3d::Bounds& bnds = *fm->bounds().at(size_t(i));
    _ui->manifoldXLabel->setText( QString::number( bnds.xlen(), 'f', 2));
    _ui->manifoldYLabel->setText( QString::number( bnds.ylen(), 'f', 2));
    _ui->manifoldZLabel->setText( QString::number( bnds.zlen(), 'f', 2));
    fm->unlock();

    emit onSelectedManifoldChanged(realIdx);
}   // end doOnManifoldIndexChanged
