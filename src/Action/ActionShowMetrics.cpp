/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
#include <MetricVisualiser.h>
#include <Ethnicities.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <algorithm>
#include <QDebug>
using FaceTools::Action::ActionShowMetrics;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MCSet;
using FaceTools::Metric::MC;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FMV;
using FaceTools::FM;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using FaceTools::Widget::MetricsDialog;
using FaceTools::Metric::MetricValue;
using MS = FaceTools::Action::ModelSelector;


double ActionShowMetrics::s_opacity(1.0);

void ActionShowMetrics::setOpacityOnShow( double v) { s_opacity = std::min( 1.0, std::max( 0.1, v));}


ActionShowMetrics::ActionShowMetrics( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _mdialog(nullptr), _nowShowing(false)
{
    setCheckable( true, _nowShowing);
    addTriggerEvent( Event::METRICS_CHANGE);
    addTriggerEvent( Event::METADATA_CHANGE);
    addTriggerEvent( Event::ASSESSMENT_CHANGE);
    addTriggerEvent( Event::CLOSED_MODEL);
    addTriggerEvent( Event::MODEL_SELECT);
}   // end ctor


void ActionShowMetrics::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _mdialog = new MetricsDialog( p);
    connect( _mdialog, &MetricsDialog::accepted, [this](){ setChecked(false); this->execute( Event::USER);});
    connect( _mdialog, &MetricsDialog::onSelectedMetric, this, &ActionShowMetrics::_doOnSelectedMetric);
    connect( _mdialog, &MetricsDialog::onSetMetricGrowthData, this, &ActionShowMetrics::_doOnSetMetricGrowthData);
    connect( _mdialog, &MetricsDialog::onChangedMetricVisibility, this, &ActionShowMetrics::_doOnChangedMetric);
    for ( FMV* fmv : MS::viewers())
        _addViewer(fmv);
}   // end postInit


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


void ActionShowMetrics::setShowScanInfoAction( QAction* a) { _mdialog->setShowScanInfoAction(a);}


void ActionShowMetrics::_doOnSetMetricGrowthData()
{
    _doOnSelectedMetric( MCM::currentMetric()->id());
    emit onEvent( Event::METRICS_CHANGE);
}   // end _doOnSetMetricGrowthData


void ActionShowMetrics::_doOnSelectedMetric( int mid)
{
    MC::Ptr pmc = MCM::previousMetric();  // Previous active metric
    for ( const FV* fv : pmc->visualiser()->applied())
        pmc->visualiser()->setHighlighted( fv, false);
    _mdialog->highlightRow( mid);
    _doOnChangedMetric( mid);
}   // end _doOnSelectedMetric


void ActionShowMetrics::_doOnChangedMetric( int mid)
{
    MC::Ptr mc = MCM::metric(mid);
    Vis::MetricVisualiser* mvis = mc->visualiser();
    const bool isCurrentMetric = MCM::currentMetric()->id() == mid;
    const bool isVisible = mc->isVisible() && isChecked();

    // For visualisable metrics, refresh their visibility/highlight state for the attached views.
    if ( mvis)
    {
        for ( FV* fv : mvis->applied())
        {
            mvis->setVisible( fv, isVisible);
            mvis->setHighlighted( fv, isCurrentMetric);
            mvis->checkState( fv);
        }   // end for
    }   // end if

    if ( isCurrentMetric)
        _updateMetricText( mid);
    MS::updateRender();
}   // end _doOnChangedMetric


bool ActionShowMetrics::doBeforeAction( Event)
{
    return true;
}   // end doBeforeAction


bool ActionShowMetrics::checkState( Event)
{
    // Ensure color of text complements viewer background
    for ( auto& p : _texts)
    {
        QColor bg = p.first->backgroundColour();
        QColor fg = chooseContrasting( bg);
        vtkTextProperty* tp = p.second->GetTextProperty();
        tp->SetBackgroundColor( bg.redF(), bg.greenF(), bg.blueF());
        tp->SetColor( fg.redF(), fg.greenF(), fg.blueF());
    }   // end for

    // Also update colours of visualisation actors to complement background.
    for ( const MC::Ptr& mc : MCM::vmetrics())
    {
        Vis::MetricVisualiser* mvis = mc->visualiser();
        for ( FV* fv : mvis->applied())
            mvis->checkState( fv);
    }   // end for

    _updateMetricText( MCM::currentMetric()->id());
    return !_mdialog->isHidden();
}   // end checkState


bool ActionShowMetrics::checkEnable( Event) { return true;}


void ActionShowMetrics::doAction( Event)
{
    // Was previously shown?
    const bool wasShowing = _nowShowing;

    const MCSet& mcs = MCM::vmetrics();
    const FM* fm = MS::selectedModel();

    if ( !isChecked())
    {
        // Hide all the metric visualisations
        for ( MC::Ptr mc : mcs)
        {
            Vis::MetricVisualiser* mvis = mc->visualiser();
            for ( FV* fv : mvis->applied())
            {
                mvis->setVisible( fv, false);
                fv->setOpacity( 1.0);
            }   // end for
        }   // end for

        _nowShowing = false;
        emit onEvent( Event::VIEW_CHANGE);
        _mdialog->hide();
    }   // end if
    else if ( fm && isChecked())
    {
        for ( MC::Ptr mc : mcs)
        {
            Vis::MetricVisualiser* mvis = mc->visualiser();

            if ( fm->currentAssessment()->hasMetric(mc->id()))
            {
                for ( FV* fv : fm->fvs())
                {
                    fv->apply( mvis);
                    if ( !_nowShowing)
                        fv->setOpacity( std::min( fv->opacity(), s_opacity));
                }   // end for
            }   // end if
        }   // end for
        _nowShowing = true;
    }   // end if

    // Note that _nowShowing is only set true if there's a model selected.
    if ( isChecked() && !wasShowing)
        _mdialog->show();

    // Refreshing here also refreshes the ChartDialog which refreshes the options
    // available for the currently selected metric's growth curve data and also
    // sets the current growth curve data for the metric to use.
    _mdialog->refresh();
}   // end doAction


void ActionShowMetrics::purge( const FM* fm, Event e)
{
    for ( MC::Ptr mc : MCM::vmetrics())
    {
        Vis::MetricVisualiser* mvis = mc->visualiser();
        for ( FV* fv : fm->fvs())
            fv->purge( mvis, e);  // Calls mvis->purge(fv, e)
    }   // end for
}   // end purge


void ActionShowMetrics::_updateMetricText( int mid)
{
    // Hide all the text actors
    for ( const auto& p : _texts)
        p.second->SetVisibility(false);

    const FM* fm = MS::selectedModel();
    if ( !fm)
        return;

    // Get the correct metric value based on laterality
    const MetricValue* mv = nullptr;
    const MetricValue* mvl = nullptr;
    const MetricValue* mvr = nullptr;

    FaceAssessment::CPtr ass = fm->currentAssessment();
    //std::cerr << "Switched to assessment " << ass->assessor().toStdString() << std::endl;

    MC::Ptr mc = MCM::metric( mid);
    if ( !mc->isBilateral())
    {
        if ( ass->cmetrics().has( mid))
            mv = &ass->cmetrics().metric(mid);
        if ( !mv)
            return;
    }   // end if
    else
    {
        if ( ass->cmetricsL().has(mid))
            mvl = &ass->cmetricsL().metric( mid);
        if ( ass->cmetricsR().has(mid))
            mvr = &ass->cmetricsR().metric( mid);
        if ( !mvl || !mvr)
            return;
    }   // end else

    const int nds = int(mc->numDecimals());
    std::ostringstream oss;
    oss << mc->name().toStdString() << (mc->isBilateral() ? " (L;R;Mean)" : "") << "\n";
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

    Metric::GrowthData::CPtr gd = mc->currentGrowthData();

    if ( !gd)
        oss << " [No growth curve data available]";
    else
    {
        for ( size_t i = 0; i < dims; ++i)
        {
            const int fw = fws[i];
            if ( mv)
            {
                oss << std::right << std::setw(fw);
                oss << mv->zscore( fm->age(), i);
            }   // end if
            else
            {
                const double zsl = mvl->zscore( fm->age(), i);
                const double zsr = mvr->zscore( fm->age(), i);
                const double zsm = 0.5 * (zsl + zsr);
                oss << std::right << std::setw(fw) << zsl << "; "
                                  << std::setw(fw) << zsr << "; "
                                  << std::setw(fw) << zsm;
            }   // end if
        }   // end for

        // Get warnings
        QStringList dwarns;

        if (( gd->sex() != (FEMALE_SEX | MALE_SEX)) && gd->sex() != fm->sex())
            dwarns << "Sex";

        QStringList epars;
        if ( !Ethnicities::belongs( gd->ethnicity(), fm->maternalEthnicity()))
            epars << "Maternal";
        if ( !Ethnicities::belongs( gd->ethnicity(), fm->paternalEthnicity()))
            epars << "Paternal";
        if ( epars.size() == 2)
        {
            if ( fm->maternalEthnicity() != fm->paternalEthnicity())
                dwarns << "Ethnicities";
            else
                dwarns << "Ethnicity";
        }   // end if
        else if ( epars.size() == 1)
            dwarns << QString( "%1 Ethnicity").arg( epars.first());

        if ( !dwarns.empty())
            oss << QString( " [Incompatible %1]").arg( dwarns.join(" & ")).toStdString();

        // Show that the subject's age is outside bounds only if the more severe mismatches aren't shown
        if ( !gd->isWithinAgeRange(fm->age()) && dwarns.empty())
            oss << " [Age Outside Bounds]";
    }   // end else

    const bool showText = _mdialog->isVisible() && MS::isViewSelected();
    const std::string ostr = oss.str();
    for ( const auto& p : _texts)
    {
        p.second->SetInput( ostr.c_str());
        p.second->SetVisibility( showText);
    }   // end for
}   // end _updateMetricText
