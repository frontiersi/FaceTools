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


MetricVisualiser::MetricVisualiser( int id)
    : BaseVisualisation( "Metric"), _id(id), _text( vtkTextActor::New())
{
    _text->GetTextProperty()->SetJustificationToLeft();
    _text->GetTextProperty()->SetFontFamilyToCourier();
    _text->GetTextProperty()->SetFontSize(17);
    _text->SetDisplayPosition( 10, 10);
    _text->SetPickable(false);

    QColor tcol(255,255,255);
    _text->GetTextProperty()->SetColor( tcol.redF(), tcol.greenF(), tcol.blueF());
    _text->SetVisibility(false);
}   // end ctor


MetricVisualiser::~MetricVisualiser() { _text->Delete();}


void MetricVisualiser::apply( FV* fv, const QPoint*) { fv->viewer()->add(_text);}
void MetricVisualiser::clear( FV* fv) { fv->viewer()->remove(_text);}
void MetricVisualiser::setCaptionsVisible( bool show) { _text->SetVisibility(show);}


void MetricVisualiser::updateCaptions( const FM* fm)
{
    // Get the correct metric value based on laterality
    const FaceTools::Metric::MetricValue* mv = nullptr;
    const FaceTools::Metric::MetricValue* mvl = nullptr;
    const FaceTools::Metric::MetricValue* mvr = nullptr;

    Metric::MC::Ptr mc = MCM::metric(_id);
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
    oss << mc->name().toStdString() << (mc->isBilateral() ? " (L;R)" : "") << "\n";
    oss << std::fixed << std::setprecision(nds);

    oss << "Measure";
    const size_t dims = mc->dims();
    oss << (dims > 1 ? "s: " : ": ");

    std::vector<int> fws(dims);  // Field widths for alignment
    for ( size_t i = 0; i < dims; ++i)
    {
        std::ostringstream voss;    // Just used to check required space for text
        if ( mv)
            voss << std::fixed << std::setprecision(nds) << mv->values()[i];
        else
            voss << std::fixed << std::setprecision(nds) << mvl->values()[i];

        fws[i] = int(voss.str().size()) + 1;

        if ( mv)
            oss << std::right << std::setw(fws[i]) << mv->values()[i];
        else
        {
            oss << std::right << std::setw(fws[i]) << mvl->values()[i] << "; ";
            oss << std::right << std::setw(fws[i]) << mvr->values()[i];
        }   // end else
    }   // end for

    bool ageInStats = true;
    oss << "\nZ-score"; // Z-scores on line below
    oss << (dims > 1 ? "s: " : ": ");
    for ( size_t i = 0; i < dims; ++i)
    {
        const int fw = fws[i];

        if ( mv)
        {
            oss << std::right << std::setw(fw);
            double zs = mv->zscores()[i];
            if ( zs < DBL_MAX)
                oss << zs;
            else
                oss << "N/A";

            if ( !mv->zokays()[i])
                ageInStats = false;
        }   // end if
        else
        {
            double zsl = mvl->zscores()[i];
            double zsr = mvr->zscores()[i];
            if ( zsl < DBL_MAX && zsr < DBL_MAX)
                oss << std::right << std::setw(fw) << zsl << "; " << std::setw(fw) << zsr;
            else
                oss << std::right << std::setw(fw) << "N/A";

            if ( !mvl->zokays()[i] || !mvr->zokays()[i])
                ageInStats = false;
        }   // end if
    }   // end for

    if ( !ageInStats)
        oss << " (TRUNCATED Z-SCORE" << (dims > 1 ? "S!)" : "!)");

    _text->SetInput( oss.str().c_str());
}   // end updateCaptions
