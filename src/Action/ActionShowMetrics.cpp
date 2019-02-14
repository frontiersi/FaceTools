/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <ActionShowMetrics.h>
#include <MetricCalculatorManager.h>
#include <PhenotypeManager.h>
#include <MetricVisualiser.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <algorithm>
#include <QDebug>
using FaceTools::Action::ActionShowMetrics;
using FaceTools::Action::FaceAction;
using FaceTools::Interactor::MetricsInteractor;
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MCSet;
using FaceTools::Metric::MC;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using FaceTools::Widget::ChartDialog;
using FaceTools::Widget::MetricsDialog;
using FaceTools::Widget::PhenotypesDialog;
using FaceTools::Action::ModelSelector;
using FaceTools::Metric::PhenotypeManager;
using FaceTools::Metric::Phenotype;


ActionShowMetrics::ActionShowMetrics( const QString& dn, const QIcon& ico, QWidget *parent)
    : FaceAction( dn, ico),
      _mdialog( new MetricsDialog( parent)),
      _cdialog( new ChartDialog( parent)),
      _pdialog( new PhenotypesDialog( parent))
{
    setCheckable( true, false);

    TestFVSTrue pfp = [this](const FVS&){ return this->isChecked();};
    setRespondToEvent( METRICS_CHANGE, pfp);

    for ( MC::Ptr mc : MCM::metrics())
        connect( &*mc, &MC::updated, this, &ActionShowMetrics::doOnMetricUpdated);  // E.g. whether metric is hidden, name change etc.

    for ( MC::Ptr mc : MCM::metrics())
        connect( &*mc, &MC::selected, this, &ActionShowMetrics::doOnSetSelectedMetric);

    connect( _mdialog, &MetricsDialog::onShowChart, this, &ActionShowMetrics::doOnShowChartDialog);
    connect( _mdialog, &MetricsDialog::onShowPhenotypes, this, &ActionShowMetrics::doOnShowPhenotypesDialog);
    connect( _mdialog, &MetricsDialog::onRefreshedMetrics, this, &ActionShowMetrics::doOnRefresh);

    connect( _mdialog, &MetricsDialog::onEthnicityIgnored, _cdialog, &ChartDialog::doOnSetEthnicityIgnored);
    connect( _mdialog, &MetricsDialog::onShowingPhenotypes, _pdialog, &PhenotypesDialog::doOnShowPhenotypes);

    _mdialog->setShowMetricsAction(qaction());
}   // end ctor


void ActionShowMetrics::setShowScanInfoAction( QAction* a) { _mdialog->setShowScanInfoAction(a);}


namespace {
void showDialog( QDialog *dialog)
{
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}   // end showDialog
}   // end namespace


void ActionShowMetrics::doOnShowChartDialog() { showDialog( _cdialog);}
void ActionShowMetrics::doOnShowPhenotypesDialog() { showDialog( _pdialog);}


void ActionShowMetrics::doOnSetSelectedMetric()
{
    MC::Ptr pmc = MCM::previousMetric();  // Previous active metric
    MC::Ptr amc = MCM::currentMetric();   // Newly active metric
    if ( pmc && pmc != amc)
    {
        pmc->visualiser()->setHighlighted(nullptr);
        pmc->visualiser()->showText(nullptr);   // Remove text of old active from all views
    }   // end if

    assert(amc);
    doOnMetricUpdated(amc->id());
}   // end doOnSetSelectedMetric



void ActionShowMetrics::doOnMetricUpdated( int mid)
{
    MC::Ptr mc = MCM::metric(mid);
    for ( FM* fm : _vmodels) // Update mc's visualiser on all currently visualised models.
    {
        const FVS& fvs = fm->fvs();
        std::for_each( std::begin(fvs), std::end(fvs), [=](FV* fv){ fv->remove(mc->visualiser());});
    }   // end for

    if ( isChecked() && mc->isVisible())
    {
        for ( FM* fm : _vmodels) // Update mc's visualiser on all currently visualised models.
        {
            for ( FV* fv : fm->fvs())
                fv->apply(mc->visualiser());
            if ( mc == MCM::currentMetric())
            {
                mc->visualiser()->setHighlighted(fm);
                mc->visualiser()->updateText(fm);
                mc->visualiser()->showText(fm);
            }   // end if
        }   // end for
    }   // end if

    FaceTools::updateRenderers( _vmodels);
}   // end doOnMetricUpdated


bool ActionShowMetrics::doOnRefresh()
{
    if ( !ModelSelector::selected())
        return false;

    size_t numShownMetrics = 0;
    FM* fm = ModelSelector::selected()->data();
    const MCSet& mcs = MCM::vmetrics();
    for ( FV* fv : fm->fvs())
    {
        for ( MC::Ptr mc : mcs)
        {
            fv->remove( mc->visualiser());
            mc->visualiser()->purge(fv);
            if ( mc->isVisible() && isChecked())
            {
                fv->apply( mc->visualiser());
                numShownMetrics++;
            }   // end if
        }   // end for
    }   // end for

    doOnSetSelectedMetric();
    fm->updateRenderers();

    return numShownMetrics > 0;
}   // end doOnRefresh


bool ActionShowMetrics::testIfCheck( const FV* fv) const
{
    if ( !fv)
        return false;
    for ( MC::Ptr mc : MCM::vmetrics())
    {
        if ( fv->isApplied( mc->visualiser()))
            return true;
    }   // end for
    return false;
}   // end testIfCheck


namespace {
bool canVisualiseAny( const MetricSet& mset)
{
    for ( int id : mset.ids())
    {
        MC::Ptr mc = MCM::metric(id);
        if ( mc->visualiser())
            return true;
    }   // end for
    return false;
}   // end canVisualiseAny
}   // end namespace


bool ActionShowMetrics::testReady( const FV* fv)
{
    const FM* fm = fv->data();
    // Ready only if the model has metrics set that can be visualised
    fm->lockForRead();
    const bool canShow = canVisualiseAny( fm->cmetrics())
                      || canVisualiseAny( fm->cmetricsL())
                      || canVisualiseAny( fm->cmetricsR());
    fm->unlock();
    return canShow;
}   // end testReady


void ActionShowMetrics::tellReady( const FV* fv, bool isready)
{
    if ( _cdialog->isVisible())
        _cdialog->refresh();
    if ( fv && isready)
        doOnSetSelectedMetric();
    _mdialog->setDetectedOnlyEnabled(fv && isready);
}   // end tellReady


bool ActionShowMetrics::testEnabled( const QPoint*) const { return ready1();}


bool ActionShowMetrics::doAction( FVS& fvs, const QPoint&)
{
    if ( !ModelSelector::selected())
        return false;

    fvs.clear();
    FM* fm = ModelSelector::selected()->data();
    fvs.insert(fm);

    if ( isChecked())
        _vmodels.insert(fm);

    _mdialog->refresh();

    /*
    // Won't stay checked if no metrics were shown!
    if ( isChecked() && !showingMetrics)
        setChecked(false);
    */

    // Ensure the metrics dialog is always shown when the metrics are.
    //if ( isChecked())
    //  showDialog(_mdialog);

    return true;
}   // end doAction


void ActionShowMetrics::purge( const FM* fm)
{
    for ( FV* fv : fm->fvs())
    {
        for ( MC::Ptr mc : MCM::vmetrics())
        {
            fv->remove( mc->visualiser());
            mc->visualiser()->purge(fv);
        }   // end for
    }   // end for
    _vmodels.erase(const_cast<FM*>(fm));
    _cdialog->refresh();
}   // end purge
