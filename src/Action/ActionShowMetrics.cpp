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

#include <Action/ActionShowMetrics.h>
#include <Action/ActionSetParallelProjection.h>
#include <Action/ActionUpdateMeasurements.h>
#include <Metric/MetricManager.h>
#include <Metric/StatsManager.h>
#include <Interactor/LandmarksHandler.h>
#include <FileIO/FaceModelManager.h>
#include <Vis/MetricVisualiser.h>
#include <Ethnicities.h>
#include <FaceTools.h>
#include <vtkTextProperty.h>
using FaceTools::Action::ActionShowMetrics;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MC;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FM;
using FaceTools::Metric::MetricValue;
using FMM = FaceTools::FileIO::FaceModelManager;
using MM = FaceTools::Metric::MetricManager;
using MS = FaceTools::ModelSelect;
using SM = FaceTools::Metric::StatsManager;

// static
bool ActionShowMetrics::s_showParallelProjection(false);

void ActionShowMetrics::setParallelProjectionOnShow( bool v) { s_showParallelProjection = v;}


ActionShowMetrics::ActionShowMetrics( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _mdialog(nullptr), _cdialog(nullptr)
{
    setCheckable( true, false);
    addRefreshEvent( Event::MODEL_SELECT | Event::METRICS_CHANGE | Event::VIEWER_CHANGE);
    addTriggerEvent( Event::CLOSED_MODEL);
}   // end ctor


void ActionShowMetrics::postInit()
{
    // Connect up events from the dialog to this control class
    QWidget* p = static_cast<QWidget*>(parent());
    _mdialog = new Widget::MetricsDialog( p);
    _cdialog = new Widget::ChartDialog( p);
    connect( _mdialog, &Widget::MetricsDialog::accepted, [this](){ setChecked(false);});
    connect( _mdialog, &Widget::MetricsDialog::onShowChart, [this](){ _cdialog->setVisible(true);});
    connect( _mdialog, &Widget::MetricsDialog::onRemeasure, this, &ActionShowMetrics::_doOnRemeasure);
    connect( _mdialog, &Widget::MetricsDialog::onMatchHPOs, this, &ActionShowMetrics::_doOnMatchHPOs);
    connect( _mdialog, &Widget::MetricsDialog::onSelectMetric, this, &ActionShowMetrics::_doOnSelectMetric);
    connect( _mdialog, &Widget::MetricsDialog::onRefreshAllMetricsVisibility,
                            this, &ActionShowMetrics::_doRefreshAllMetricsVisibility);
    connect( _mdialog, &Widget::MetricsDialog::onRefreshMetricVisibility,
                            this, &ActionShowMetrics::_doRefreshMetricVisibility);

    // Allow the LandmarksHandler to communicate changes to landmark positions
    // requiring their associated metrics to be recalculated and graphics redrawn.
    // For performance reasons, the landmarks handler only emits onDoingDrag events
    // if the measurements dialog is visible.
    using Interactor::LandmarksHandler;
    LandmarksHandler *h = MS::handler<LandmarksHandler>();
    if ( h)
    {
        std::function<void( int, FaceSide)> fn =
            [this]( int lmid, FaceSide fs)
            {
                const IntSet &mids = MM::metricsForLandmark( lmid);
                FM::WPtr fm = MS::selectedModelScopedWrite();
                for ( int mid : mids)
                {
                    ActionUpdateMeasurements::updateMeasurement( fm.get(), mid);
                    _refreshMetricAppearance( fm.get(), mid);
                }   // end for
                if ( mids.count( _mdialog->currentMetricId()) > 0)
                    _updateCurrentMetricDisplayedInfo();
                if ( fm->fvs().size() >= 2)
                    MS::updateRender();
            };
        connect( h, &LandmarksHandler::onDoingDrag, fn);
    }   // end if

    // Add text actors to viewers
    for ( FMV* fmv : MS::viewers())
    {
        assert( _texts.count(fmv) == 0);
        vtkNew<vtkTextActor>& text = _texts[fmv];
        text->GetTextProperty()->SetJustificationToLeft();
        text->GetTextProperty()->SetFontFamilyToCourier();
        text->GetTextProperty()->SetFontSize(14);
        text->GetTextProperty()->SetBackgroundOpacity(0.8);
        text->GetTextProperty()->SetBold(false);
        text->SetDisplayPosition( 6, 3);
        text->SetPickable(false);
        text->SetVisibility(false);
        fmv->add( _texts.at(fmv));
    }   // end for
}   // end postInit


void ActionShowMetrics::_doOnRemeasure()
{
    const int mid = _mdialog->currentMetricId();
    for ( FM *fm : FMM::opened())
    {
        fm->lockForWrite();
        ActionUpdateMeasurements::updateMeasurement( fm, mid);
        _refreshMetricAppearance( fm, mid);
        fm->unlock();
    }   // end for
    _updateCurrentMetricDisplayedInfo();
    MS::updateRender();
}   // end _doOnRemeasure


void ActionShowMetrics::_refreshMetricAppearance( FM *fm, int mid)
{
    const MC *metric = MM::cmetric(mid);
    Vis::MetricVisualiser *mvis = metric->visualiser();
    for ( FV *fv : fm->fvs())
        if ( mvis->isVisible(fv))
            mvis->refresh( fv);
}   // end _refreshMetricAppearance


void ActionShowMetrics::_updateCurrentMetricDisplayedInfo()
{
    const MC *metric = MM::cmetric( _mdialog->currentMetricId());
    _cdialog->refresh( metric);
    const bool isVis = metric && metric->isVisible();
    if ( metric)
        metric->visualiser()->setHighlighted( true);
    for ( auto& p : _texts) // Update text on each viewer and set text actor visibility
    {
        const FMV *fmv = p.first;
        const FV *fv = fmv->selected();
        bool showTextOnViewer = isVis && fv && fmv->attached().size() == 1;
        if ( showTextOnViewer)
            showTextOnViewer &= _updateText( fv, metric->id());
        p.second->SetVisibility( showTextOnViewer);
    }   // end for
}   // end _updateCurrentMetricDisplayedInfo


void ActionShowMetrics::_doOnSelectMetric( int omid, int nmid)
{
    if ( omid >= 0)
        MM::cmetric( omid)->visualiser()->setHighlighted( false);
    if ( nmid >= 0)
        MM::cmetric( nmid)->visualiser()->setHighlighted( true);
    _updateCurrentMetricDisplayedInfo();
    MS::updateRender();
}   // end _doOnSelectMetric


void ActionShowMetrics::_doOnMatchHPOs()
{
    _doRefreshAllMetricsVisibility();
    _updateCurrentMetricDisplayedInfo();
    MS::updateRender();
}   // end _doOnMatchHPOs


void ActionShowMetrics::_doRefreshAllMetricsVisibility()
{
    for ( int mid : MM::ids())
        _refreshMetricVisibility( mid);
    MS::updateRender();
}   // end _doRefreshAllMetricsVisibility


void ActionShowMetrics::_doRefreshMetricVisibility( int mid)
{
    _refreshMetricVisibility( mid);
    MS::updateRender();
}   // end _doRefreshMetricVisibility


void ActionShowMetrics::_refreshMetricVisibility( int mid)
{
    const MC *metric = MM::cmetric(mid);
    if ( metric)
    {
        Vis::MetricVisualiser *mvis = metric->visualiser();
        const bool isVis = metric->isVisible() && _mdialog->isVisible();
        for ( FM *fm : FMM::opened())
        {
            for ( FV *fv : fm->fvs())
            {
                if ( isVis && mvis->isAvailable( fv))
                    fv->apply( mvis);
                else
                    mvis->setVisible( fv, false);
            }   // end for
        }   // end for
    }   // end if
}   // end _refreshMetricVisibility


bool ActionShowMetrics::update( Event e)
{
    if ( _mdialog->isVisible())
        _mdialog->refreshMatched(); // Calls _doOnMatchHPOs via emitted signal
    return _mdialog->isVisible();
}   // end update


bool ActionShowMetrics::isAllowed( Event)
{
    for ( const FM *fm : FMM::opened())
        if ( fm->hasLandmarks())
            return true;
    return false;
}   // end isAllowed


void ActionShowMetrics::_setParallelProjection( bool v)
{
    if ( s_showParallelProjection)
        ActionSetParallelProjection::setParallelProjection( v);
}   // end _setParallelProjection


void ActionShowMetrics::doAction( Event e)
{
    if ( isChecked() && MS::isViewSelected())
    {
        _mdialog->show();
        _doRefreshAllMetricsVisibility();
    }   // end if
    else
    {
        _mdialog->hide();
        _cdialog->hide();
        _doRefreshAllMetricsVisibility();
        for ( auto& p : _texts)
            p.second->SetVisibility( false);
    }   // end else if

    _setParallelProjection( _mdialog->isVisible());
    using Interactor::LandmarksHandler;
    LandmarksHandler *lmksHandler = MS::handler<LandmarksHandler>();
    if ( lmksHandler)
        lmksHandler->setEmitOnDrag( _mdialog->isVisible());
}   // end doAction


Event ActionShowMetrics::doAfterAction( Event)
{
    Event e = Event::VIEW_CHANGE | Event::ALL_VIEWS | Event::ALL_VIEWERS;
    if ( s_showParallelProjection)
        e |= Event::CAMERA_CHANGE;
    return e;
}   // end doAfterAction


bool ActionShowMetrics::_updateText( const FV *fv, int mid)
{
    assert(fv);
    const FM *fm = fv->data();
    FaceAssessment::CPtr ass = fm->currentAssessment();
    if ( !ass->hasMetric( mid))
        return false;

    const MC *mc = MM::cmetric(mid);
    assert( mc);

    // Get the correct metric value based on laterality
    const MetricValue* mv = nullptr;
    const MetricValue* mvr = nullptr;
    const MetricValue* mvl = nullptr;

    if ( !mc->isBilateral())
    {
        if ( ass->cmetrics(MID).has( mid))
            mv = &ass->cmetrics(MID).metric(mid);
    }   // end if
    else
    {
        if ( ass->cmetrics(RIGHT).has(mid))
            mvr = &ass->cmetrics(RIGHT).metric( mid);
        if ( ass->cmetrics(LEFT).has(mid))
            mvl = &ass->cmetrics(LEFT).metric( mid);
    }   // end else

    std::string units;
    if ( !mc->units().isEmpty())
        units = " [" + mc->units().toStdString() + "]";

    SM::RPtr gd = SM::stats( mid, fm);

    std::string inplane;
    if ( !mc->fixedInPlane())
    {
        const bool inp = mc->inPlane(fm);
        if ( gd && inp != gd->inPlane())
            inplane = " [IN-PLANE MISMATCH]";
        else if ( inp)
            inplane = " [IN-PLANE]";
    }   // end if

    const size_t nds = mc->numDecimals();
    std::ostringstream oss;
    oss << mc->name().toStdString() << units << inplane << "\n";
    oss << std::fixed << std::setprecision(nds);

    oss << "Measure";
    const size_t dims = mc->dims();
    oss << (dims > 1 ? "s: " : ": ");

    std::vector<int> fws(dims);  // Field widths for alignment
    for ( size_t i = 0; i < dims; ++i)
    {
        std::ostringstream voss;    // Just used to check required space for text
        if ( mv)
            voss << std::fixed << std::setprecision(nds) << fabs(mv->value(i));
        else
            voss << std::fixed << std::setprecision(nds) << std::max<float>(fabs(mvl->value(i)), fabs(mvr->value(i)));
        fws[i] = int(voss.str().size()) + 2;

        if ( mv)
            oss << std::right << std::setw(fws[i]) << mv->value(i);
        else
        {
            oss << std::right << std::setw(fws[i]) << mvr->value(i) << " (R)";
            oss << std::right << std::setw(fws[i]) << mvl->value(i) << " (L)";
            oss << std::right << std::setw(fws[i]) << (0.5 * (mvl->value(i) + mvr->value(i))) << " (Mean)";
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
                    const double zsr = mvr->zscore( age, i);
                    const double zsl = mvl->zscore( age, i);
                    const double zsm = 0.5 * (zsl + zsr);
                    oss << std::right << std::setw(fw) << zsr << " (R)"
                                      << std::setw(fw) << zsl << " (L)"
                                      << std::setw(fw) << zsm << " (Mean)";
                }   // end if
                else
                {
                    oss << std::right << std::setw(fw) << "----"
                                      << std::setw(fw) << "----"
                                      << std::setw(fw) << "----";
                }   // end else
            }   // end if
        }   // end for

        if ( age == 0.0f)
            oss << " [DOB unset]";
        else if ( !gd->isWithinAgeRange(age))
            oss << " [Age Outside Inferential Domain!]";
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
