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
#include <FaceModel.h>
#include <FaceTools.h>
#include <QTools/QImageTools.h>
#include <QPushButton>
#include <QSpinBox>
#include <cmath>
#include <cassert>
using FaceTools::Widget::MeshInfoDialog;
using FaceTools::FM;


MeshInfoDialog::MeshInfoDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::MeshInfoDialog),
    _model(nullptr),
    _dialogRootTitle( parent->windowTitle() + " | Model Properties")
{
    _ui->setupUi(this);
    _ui->spatialGroupBox->setTitle( QString("Spatial Extents (%1)").arg( FM::LENGTH_UNITS));
    setWindowTitle( _dialogRootTitle);
    connect( _ui->manifoldSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                             this, &MeshInfoDialog::doOnManifoldIndexChanged);
    setFixedSize( geometry().width(), geometry().height());
    reset();
}   // end ctor


MeshInfoDialog::~MeshInfoDialog()
{
    delete _ui;
}   // end dtor


void MeshInfoDialog::set( const FM* fm)
{
    _model = fm;
    reset();
}   // end set


// private
void MeshInfoDialog::reset()
{
    setWindowTitle( _dialogRootTitle);
    _ui->modelPolygonsLabel->clear();
    _ui->modelVerticesLabel->clear();
    _ui->modelBoundariesLabel->clear();

    int nm = 0; // Number of manifolds

    if ( _model)
    {
        _model->lockForRead();
        const r3d::Manifolds& manfs = _model->manifolds();
        const r3d::Mesh& cmodel = _model->mesh();
        nm = int(manfs.count());  // Number of manifolds

        //_ui->textureMappedLabel->setText( cmodel.materialIds().empty() ? "No Texture Mapping!" : "");

        _ui->modelPolygonsLabel->setText( QString("%1").arg(cmodel.numFaces()));
        _ui->modelVerticesLabel->setText( QString("%1").arg(cmodel.numVtxs()));
        size_t nb = 0;  // Count up the total number of boundaries
        for ( int i = 0; i < nm; ++i)
            nb += manfs[i].boundaries().count();
        _ui->modelBoundariesLabel->setText( QString("%1").arg(nb));
        _model->unlock();
    }   // end if

    //_ui->maxManifoldsLabel->setText( QString("of %1").arg(nm));
    _ui->manifoldSpinBox->setMaximum( nm);
    _ui->manifoldSpinBox->setMinimum( 1);
    _ui->manifoldSpinBox->setEnabled( nm > 1);
    _ui->manifoldSpinBox->setValue( 1);
    doOnManifoldIndexChanged(1);
}   // end reset


void MeshInfoDialog::doOnManifoldIndexChanged( int i)
{
    _ui->manifoldPolygonsLabel->clear();
    _ui->manifoldVerticesLabel->clear();
    _ui->manifoldBoundariesLabel->clear();

    _ui->manifoldXLabel->clear();
    _ui->manifoldYLabel->clear();
    _ui->manifoldZLabel->clear();

    if ( _model)
    {
        _model->lockForRead();
        const r3d::Manifolds& manfs = _model->manifolds();
        assert( i > 0 && i <= int(manfs.count()));
        const int realIdx = int(i-1);

        const r3d::Manifold& manf = manfs[realIdx];
        const size_t p = manf.faces().size();
        const size_t v = manf.vertices().size();
        const size_t b = manf.boundaries().count();
        _ui->manifoldPolygonsLabel->setText( QString("%1").arg(p));
        _ui->manifoldVerticesLabel->setText( QString("%1").arg(v));
        _ui->manifoldBoundariesLabel->setText( QString("%1").arg(b));

        const r3d::Bounds& bnds = *_model->bounds().at(size_t(i));
        const double x = bnds.xlen();
        const double y = bnds.ylen();
        const double z = bnds.zlen();
        _ui->manifoldXLabel->setText( QString::number(x, 'f', 2));
        _ui->manifoldYLabel->setText( QString::number(y, 'f', 2));
        _ui->manifoldZLabel->setText( QString::number(z, 'f', 2));
        _model->unlock();

        emit selectedManifold(realIdx);
    }   // end if
}   // end doOnManifoldIndexChanged
