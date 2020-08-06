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

#include <Action/ActionShowMetrics.h>
#include <Action/ActionSetParallelProjection.h>
#include <Metric/MetricManager.h>
#include <FileIO/FaceModelManager.h>
#include <Vis/MetricVisualiser.h>
#include <Ethnicities.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <algorithm>
#include <QDebug>
#include <vtkTextProperty.h>
using FaceTools::Action::ActionShowMetrics;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MCSet;
using FaceTools::Metric::MC;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FM;
using FaceTools::Metric::MetricValue;
using FMM = FaceTools::FileIO::FaceModelManager;
using MM = FaceTools::Metric::MetricManager;
using MS = FaceTools::Action::ModelSelector;

// static
bool ActionShowMetrics::s_showParallelProjection(false);

void ActionShowMetrics::setParallelProjectionOnShow( bool v) { s_showParallelProjection = v;}


ActionShowMetrics::ActionShowMetrics( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _mdialog(nullptr), _cdialog(nullptr), _pmid(-1)
{
    setCheckable( true, false);
    addTriggerEvent( Event::METRICS_CHANGE | Event::CLOSED_MODEL | Event::RESTORE_CHANGE | Event::VIEWER_CHANGE);
}   // end ctor


void ActionShowMetrics::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _mdialog = new Widget::MetricsDialog( p);
    _cdialog = new Widget::ChartDialog( p);
    connect( _mdialog, &Widget::MetricsDialog::onShowChart, this, &ActionShowMetrics::_doOnShowChart);
    connect( _mdialog, &Widget::MetricsDialog::accepted, [this](){ _closeDialog( Event::USER);});
    // Emitting STATS_CHANGE will always result in Event::METRICS_CHANGE being emitted from ActionUpdateMeasurements
    connect( _mdialog, &Widget::MetricsDialog::onStatsChanged, [this](){ emit onEvent( Event::STATS_CHANGE);});
    connect( _mdialog, &Widget::MetricsDialog::onRefreshAllMetrics, this, &ActionShowMetrics::_doRefreshGraphics);
    for ( FMV* fmv : MS::viewers())
        _addViewer(fmv);
}   // end postInit


void ActionShowMetrics::_doOnShowChart()
{
    _cdialog->show();
    _cdialog->raise();
    _cdialog->activateWindow();
}   // end _doOnShowChart


bool ActionShowMetrics::checkState( Event e)
{
    const FV *fv = MS::selectedView();
    const bool showing = _mdialog->isVisible() && fv && fv->data()->hasLandmarks();
    if ( isTriggerEvent(e))
    {
        _doRefreshGraphics();
        _mdialog->reflectCurrentMetricStats();
        _mdialog->reflectAtypical();
        _cdialog->refresh();
    }   // end if

    if ( !showing)
        _closeDialog( e);
    return showing;
}   // end checkState


bool ActionShowMetrics::isAllowed( Event)
{
    const FV *fv = MS::selectedView();
    return fv && fv->data()->hasLandmarks();
}   // end isAllowed


void ActionShowMetrics::_addViewer( FMV* fmv)
{
    assert( _texts.count(fmv) == 0);
    vtkNew<vtkTextActor>& text = _texts[fmv];
    text->GetTextProperty()->SetJustificationToLeft();
    text->GetTextProperty()->SetFontFamilyToCourier();
    text->GetTextProperty()->SetFontSize(20);
    text->GetTextProperty()->SetBackgroundOpacity(0.7);
    text->SetDisplayPosition( 8, 3);
    text->SetPickable(false);
    text->SetVisibility(false);
    fmv->add( _texts.at(fmv));
}   // end _addViewer


void ActionShowMetrics::_setMetricHighlighted( int mid, bool v)
{
    if ( _pmid >= 0)    // Previous metric
    {
        const MC::Ptr mc = MM::metric( _pmid);
        Vis::MetricVisualiser *mvis = mc->visualiser();
        for ( const FV *fv : mvis->applied())
            mvis->setHighlighted( fv, false);
    }   // end if

    const MC::Ptr mc = MM::metric(mid);
    if ( mc && mc->visualiser())
    {
        Vis::MetricVisualiser *mvis = mc->visualiser();
        for ( const FV *fv : mvis->applied())
            mvis->setHighlighted( fv, v);
        _pmid = mid;
    }   // end if
}   // end _setMetricHighlighted


void ActionShowMetrics::_doRefreshGraphics()
{
    const bool isShowing = _mdialog->isVisible();
    for ( MC::Ptr mc : MM::visMetrics())
    {
        if ( isShowing && mc->isVisible())
        {
            for ( const FM *fm : FMM::opened())
                for ( FV* fv : fm->fvs())
                    fv->apply( mc->visualiser());
        }   // end if
        else
        {
            for ( const FM *fm : FMM::opened())
                for ( FV* fv : fm->fvs())
                    mc->visualiser()->setVisible( fv, false);
        }   // end else
    }   // end for

    // Refresh current
    int mid = -1;
    if ( MM::currentMetric())
        mid = MM::currentMetric()->id();
    const bool showCurrent = isShowing && mid >= 0 && MM::currentMetric()->isVisible();
    for ( auto& p : _texts)
    {
        const FMV *fmv = p.first;
        const bool showTextOnViewer = showCurrent && fmv->attached().size() == 1 && _updateText( fmv->attached().first(), mid);
        p.second->SetVisibility( showTextOnViewer);
    }   // end for

    _setMetricHighlighted( mid, true);

    MS::updateRender();
}   // end _doRefreshGraphics


void ActionShowMetrics::doAction( Event e)
{
    if ( isChecked())
    {
        if ( !_mdialog->isVisible())
        {
            _mdialog->show();   // Will cause onStatsChanged to be emitted
            _doRefreshGraphics(); // Has to happen here as well as checkState
            _setParallelProjection(true);
        }   // end if
    }   // end if
    else if ( _mdialog->isVisible())
        _closeDialog( e);
}   // end doAction


void ActionShowMetrics::_closeDialog( Event e)
{
    _mdialog->hide();
    _cdialog->hide();
    _setParallelProjection(false);
    setChecked(false);
    _hideGraphics();
    doAfterAction( e);
}   // end _closeDialog


Event ActionShowMetrics::doAfterAction( Event)
{
    Event e = Event::VIEW_CHANGE | Event::ALL_VIEWS | Event::ALL_VIEWERS;
    if ( s_showParallelProjection)
        e |= Event::CAMERA_CHANGE;
    return e;
}   // end doAfterAction


void ActionShowMetrics::_hideGraphics()
{
    for ( MC::Ptr mc : MM::visMetrics())
        for ( const FM *fm : FMM::opened())
            for ( FV *fv : fm->fvs())
                mc->visualiser()->setVisible( fv, false);
    for ( auto& p : _texts)
        p.second->SetVisibility( false);
}   // end _hideGraphics


void ActionShowMetrics::_setParallelProjection( bool v)
{
    if ( s_showParallelProjection && isChecked())
        ActionSetParallelProjection::setParallelProjection( v);
}   // end _setParallelProjection


bool ActionShowMetrics::_updateText( const FV *fv, int mid)
{
    assert(fv);
    const FM* fm = fv->data();
    FaceAssessment::CPtr ass = fm->currentAssessment();
    if ( !ass->hasMetric( mid))
        return false;

    MC::Ptr mc = MM::metric(mid);

    // Get the correct metric value based on laterality
    const MetricValue* mv = nullptr;
    const MetricValue* mvr = nullptr;
    const MetricValue* mvl = nullptr;

    if ( !mc->isBilateral())
    {
        if ( ass->cmetrics().has( mid))
            mv = &ass->cmetrics().metric(mid);
    }   // end if
    else
    {
        if ( ass->cmetricsL().has(mid))
            mvl = &ass->cmetricsL().metric( mid);
        if ( ass->cmetricsR().has(mid))
            mvr = &ass->cmetricsR().metric( mid);
    }   // end else

    std::string units;
    if ( !mc->units().isEmpty())
        units = " [" + mc->units().toStdString() + "]";
    std::string bilat;
    if ( mc->isBilateral())
        bilat = " (R;L;Mean)";

    const Metric::GrowthData *gd = mc->growthData().current();

    std::string inplane;
    if ( !mc->fixedInPlane())
    {
        if ( gd && mc->inPlane() != gd->inPlane())
            inplane = " [IN-PLANE STATS MISMATCH]";
        else if ( mc->inPlane())
            inplane = " [IN-PLANE]";
    }   // end if

    const size_t nds = mc->numDecimals();
    std::ostringstream oss;
    oss << mc->name().toStdString() << units << bilat << inplane << "\n";
    oss << std::fixed << std::setprecision(nds);

    oss << "Measure";
    const size_t dims = mc->dims();
    oss << (dims > 1 ? "s: " : ": ");

    std::vector<int> fws(dims);  // Field widths for alignment
    for ( size_t i = 0; i < dims; ++i)
    {
        std::ostringstream voss;    // Just used to check required space for text
        if ( mv)
            voss << std::fixed << std::setprecision(nds) << mv->value(i);
        else
            voss << std::fixed << std::setprecision(nds) << mvl->value(i);

        fws[i] = int(voss.str().size()) + 1;

        if ( mv)
            oss << std::right << std::setw(fws[i]) << mv->value(i);
        else
        {
            oss << std::right << std::setw(fws[i]) << mvl->value(i) << "; ";
            oss << std::right << std::setw(fws[i]) << mvr->value(i) << "; ";
            oss << std::right << std::setw(fws[i]) << (0.5 * (mvl->value(i) + mvr->value(i)));
        }   // end else
    }   // end for

    oss << "\nZ-score" << (dims > 1 ? "s: " : ": ");    // Z-scores on line below

    if ( !gd)
        oss << " [Statistics N/A]";
    else
    {
        const float age = fm->age();
        for ( size_t i = 0; i < dims; ++i)
        {
            const int fw = fws[i];
            if ( mv)
            {
                oss << std::right << std::setw(fw);
                if ( age > 0.0f)
                    oss << mv->zscore( age, i);
                else
                    oss << "----";
            }   // end if
            else
            {
                if ( age > 0.0f)
                {
                    const double zsl = mvl->zscore( age, i);
                    const double zsr = mvr->zscore( age, i);
                    const double zsm = 0.5 * (zsl + zsr);
                    oss << std::right << std::setw(fw) << zsl << "; "
                                      << std::setw(fw) << zsr << "; "
                                      << std::setw(fw) << zsm;
                }   // end if
                else
                {
                    oss << std::right << std::setw(fw) << "----" << "; "
                                      << std::setw(fw) << "----" << "; "
                                      << std::setw(fw) << "----";
                }   // end else
            }   // end if
        }   // end for

        if ( age == 0.0f)
            oss << " [DOB unset]";
        else if ( !gd->isWithinAgeRange(age))
            oss << " [Age Outside Bounds]";
        else
        {
            QStringList dwarns;
            if ( gd->sex() != UNKNOWN_SEX && gd->sex() != fm->sex())
                dwarns << tr("Sex");

            QStringList epars;
            if ( !Ethnicities::belongs( gd->ethnicity(), fm->maternalEthnicity()))
                epars << tr("Maternal");
            if ( !Ethnicities::belongs( gd->ethnicity(), fm->paternalEthnicity()))
                epars << tr("Paternal");
            if ( epars.size() == 2)
                dwarns << tr("Ethnic");
            else if ( epars.size() == 1)
                dwarns << tr("%1 Ethnic").arg( epars.first());

            if ( !dwarns.empty())
                oss << tr(" [%1 Mismatch]").arg( dwarns.join(" & ")).toStdString();
        }   // end else
    }   // end else

    const std::string ostr = oss.str();
    const FMV *fmv = fv->viewer();
    _texts[fmv]->SetInput( ostr.c_str());

    // Set colours
    QColor bg = fmv->backgroundColour();
    QColor fg = chooseContrasting( bg);
    vtkTextProperty* tp = _texts[fmv]->GetTextProperty();
    tp->SetBackgroundColor( bg.redF(), bg.greenF(), bg.blueF());
    tp->SetColor( fg.redF(), fg.greenF(), fg.blueF());
    return true;
}   // end _updateText
