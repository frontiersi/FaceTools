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

#include <MetricVisualiser.h>
#include <MetricCalculatorManager.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <MetricValue.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <sstream>
#include <iomanip>
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::MetricVisualiser;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using MC = FaceTools::Metric::MetricCalculator;
using FaceTools::Metric::MetricValue;


MetricVisualiser::MetricVisualiser( int id) : BaseVisualisation( "Metric"), _id(id)
{}   // end ctor


MetricVisualiser::~MetricVisualiser()
{
    while (!_texts.empty())
        purge( const_cast<FV*>(_texts.begin()->first));
}   // end dtor


void MetricVisualiser::apply( FV* fv, const QPoint*)
{
    vtkTextActor* text = nullptr;
    if ( _texts.count(fv) == 0)
    {
        text = _texts[fv] = vtkTextActor::New();
        text->GetTextProperty()->SetJustificationToLeft();
        text->GetTextProperty()->SetFontFamilyToCourier();
        text->GetTextProperty()->SetFontSize(17);
        text->GetTextProperty()->SetBackgroundOpacity(0.7);
        text->SetDisplayPosition( 8, 3);
        text->SetPickable(false);
        static const QColor tcol(255,255,255);
        text->GetTextProperty()->SetColor( tcol.redF(), tcol.greenF(), tcol.blueF());
        text->SetVisibility(false);
    }   // end if
    text = _texts.at(fv);
    fv->viewer()->add(text);
}   // end apply


void MetricVisualiser::clear( FV* fv)
{
    if ( _texts.count(fv) > 0)
        fv->viewer()->remove( _texts.at(fv));
}   // end clear


void MetricVisualiser::purge( FV* fv)
{
    clear(fv);
    if ( _texts.count(fv) > 0)
    {
        _texts.at(fv)->Delete();
        _texts.erase(fv);
    }   // end if
}   // end purge


void MetricVisualiser::showText( const FM* fm)
{
    for ( auto& p : _texts)
        p.second->SetVisibility(false);

    if ( fm)
    {
        for ( const FV* fv : fm->fvs())
        {
            if ( _texts.count(fv) > 0)
                _texts.at(fv)->SetVisibility(true);
        }   // end for
    }   // end if
}   // end showText


void MetricVisualiser::updateText( const FM* fm)
{
    // Get the correct metric value based on laterality
    const MetricValue* mv = nullptr;
    const MetricValue* mvl = nullptr;
    const MetricValue* mvr = nullptr;

    MC::Ptr mc = MCM::metric(_id);
    if ( !mc->isBilateral())
    {
        mv = fm->metrics().get( _id);
        if (!mv)
            return;
    }   // end if
    else
    {
        mvl = fm->metricsL().get( _id);
        mvr = fm->metricsR().get( _id);
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

    const Metric::GrowthData* gd = mc->growthData(fm);
    oss << "\nZ-score" << (dims > 1 ? "s: " : ": ");    // Z-scores on line below
    if ( !gd)
        oss << " N/A (sex mismatch)";
    else
    {
        // Warn if model age outside of growth data range
        bool inAgeRange = fm->age() > 0.0;
        if ( inAgeRange)
        {
            for ( size_t i = 0; i < dims; ++i)
            {
                double minAge = gd->rsd(i)->tmin();
                double maxAge = gd->rsd(i)->tmax();
                if ( fm->age() < minAge || fm->age() > maxAge)
                {
                    inAgeRange = false;
                    break;
                }   // end if
            }   // end for
        }   // end if

        if ( !inAgeRange)
        {
            if ( fm->age() > 0)
                oss << " N/A (subject age out of range)";
            else
                oss << " N/A (set subject age > 0)";
        }   // end if
        else
        {
            for ( size_t i = 0; i < dims; ++i)
            {
                const int fw = fws[i];
                if ( mv)
                {
                    oss << std::right << std::setw(fw);
                    oss << mv->zscore(i);
                }   // end if
                else
                {
                    const double zsl = mvl->zscore(i);
                    const double zsr = mvr->zscore(i);
                    const double zsm = 0.5 * (zsl + zsr);
                    oss << std::right << std::setw(fw) << zsl << "; "
                                      << std::setw(fw) << zsr << "; "
                                      << std::setw(fw) << zsm;
                }   // end if
            }   // end for

            if ( gd->ethnicity() != fm->ethnicity())
                oss << " [Ethnic Mismatch!]";
        }   // end else
    }   // end else

    const std::string ostr = oss.str();
    for ( const FV* fv : fm->fvs())
    {
        if ( _texts.count(fv) > 0)
            _texts.at(fv)->SetInput( ostr.c_str());
    }   // end for
}   // end updateText
