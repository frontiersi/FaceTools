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

#include <Report.h>
#include <PDFGenerator.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <Ethnicities.h>
#include <PhenotypeManager.h>
#include <Chart.h>
#include <QtSvg/QSvgGenerator>
#include <QPixmap>
#include <QFile>
#include <QtDebug>
#include <MetricCalculatorManager.h>
using MCM = FaceTools::Metric::MetricCalculatorManager;
using MC = FaceTools::Metric::MetricCalculator;
using FaceTools::Metric::GrowthData;
using FaceTools::Metric::MetricValue;
using FaceTools::Metric::MetricSet;

using FaceTools::Report::Report;
using RFeatures::CameraParams;
using RFeatures::ObjModel;
using FaceTools::FM;
using FaceTools::Metric::PhenotypeManager;
using FaceTools::Metric::Phenotype;

// public
Report::Report( QTemporaryDir& tdir) : _tmpdir(tdir), _model(nullptr)
{
    _lua.new_usertype<Report>( "Report",
                               "makeFigure", &Report::makeFigure,
                               "makeChart", &Report::makeChart,
                               "makeScanInfo", &Report::makeScanInfo,
                               "showNotes", &Report::showNotes);

    _lua.new_usertype<MetricSet>( "MetricSet",
                                  "metric", &MetricSet::metric);

    _lua.new_usertype<MetricValue>( "MetricValue",
                                    "ndims", &MetricValue::ndims,
                                    "value", &MetricValue::value,
                                    "zscore", &MetricValue::zscore,
                                    "mean", &MetricValue::mean);

    _lua.new_usertype<MC>( "MC",
                           "currentGrowthData", &MC::currentGrowthData);

    _lua.new_usertype<GrowthData>( "GrowthData",
                                   "source", &GrowthData::source,
                                   "note", &GrowthData::note,
                                   "longNote", &GrowthData::longNote);

    _lua.new_usertype<FM>( "FM",
                           "age", &FM::age,
                           "sex", &FM::sex,
                           "studyId", &FM::studyId,
                           "maternalEthnicity", &FM::maternalEthnicity,
                           "paternalEthnicity", &FM::paternalEthnicity,
                           "captureDate", &FM::captureDate,
                           "dateOfBirth", &FM::dateOfBirth,
                           "currentAssessment", &FM::cassessment);

    _lua.new_usertype<FaceAssessment>( "FaceAssessment",
                           "hasLandmarks", &FaceAssessment::hasLandmarks,
                           "metrics", &FaceAssessment::cmetrics,
                           "metricsL", &FaceAssessment::cmetricsL,
                           "metricsR", &FaceAssessment::cmetricsR);

    _lua.set_function( "phenotype", PhenotypeManager::cphenotype);
    _lua.set_function( "discover", PhenotypeManager::discover);
    _lua.set_function( "metric", MCM::metric);

    _lua.new_usertype<Phenotype>( "Phenotype",
                                  "name", &Phenotype::name);

    _lua.new_usertype<QDate>( "QDate",
                              "day", &QDate::day,
                              "month", &QDate::month,
                              "year", &QDate::year);

    _lua.new_usertype<QString>( "QString",
                                "toStdString", &QString::toStdString,
                                "isEmpty", &QString::isEmpty);

    _lua.open_libraries( sol::lib::base);
    _lua.open_libraries( sol::lib::math);
}   // end ctor


// public
Report::~Report() {}


// public
bool Report::isAvailable(const FM *fm) const
{
    bool available = false;
    try
    {
        sol::function_result result = _available( fm);
        if ( result.valid())
            available = result;
    }   // end try
    catch (const sol::error& e)
    {
        qWarning() << "Lua Error!:" << e.what();
        available = false;
    }   // end catch
    return available;
}   // end isAvailable


// public
Report::Ptr Report::load( const QString& fname, QTemporaryDir& tdir)
{
    bool loadedOk = false;
    Ptr report( new Report( tdir), []( Report* d){ delete d;});

    try
    {
        QFile file(fname);
        if ( file.open(QIODevice::ReadOnly | QIODevice::Text))  // Open for read only text
        {
            QTextStream in(&file);
            const QString code = in.readAll();
            report->_lua.script( code.toStdString());
            //report->_lua.script_file( fpath.toStdString());
            loadedOk = true;
        }   // end if
    }   // end try
    catch ( const sol::error& e)
    {
        qWarning() << "Unable to load and execute file '" << fname << "'!" << endl << "\t" << e.what();
    }   // end catch

    if ( !loadedOk)
        return nullptr;

    auto table = report->_lua["report"];
    if ( !table.valid())
    {
        qWarning() << "Missing table 'report'!";
        return nullptr;
    }   // end if

    if ( sol::optional<std::string> v = table["name"])
        report->_name = v.value().c_str();
    else
    {
        qWarning() << "Missing report name!";
        return nullptr;
    }   // end else

    if ( sol::optional<std::string> v = table["title"])
        report->_title = v.value().c_str();
    else
    {
        qWarning() << "Missing report title!";
        return nullptr;
    }   // end else

    if ( sol::optional<sol::function> v = table["available"])
        report->_available = v.value();
    else
    {
        qWarning() << "Missing 'available' function!";
        return nullptr;
    }   // end else

    if ( sol::optional<sol::function> v = table["content"])
        report->_content = v.value();
    else
    {
        qWarning() << "Missing 'content' function!";
        return nullptr;
    }   // end else

    return report;
}   // end load


// public
bool Report::generate( const FM* fm, const QString& u3dfile, const QString& pdffile)
{
    QFile pfile(pdffile);
    if ( pfile.exists() && !pfile.remove())
    {
        qWarning() << "Cannot overwrite" << pdffile << "!";
        emit onFinishedGenerate( nullptr, pdffile);
        return false;
    }   // end if

    _model = fm;
    _u3dfile = u3dfile;

    QFile texfile( _tmpdir.filePath("report.tex"));
    texfile.open( QIODevice::ReadWrite | QIODevice::Text);
    if ( !texfile.isOpen())
    {
        qWarning() << "Unable to open '" << texfile.fileName() << "' for writing!";
        emit onFinishedGenerate( nullptr, pdffile);
        return false;
    }   // end if

    QTextStream os( &texfile);
    const bool writtenLatexOk = writeLatex( os);
    texfile.close();
    if ( !writtenLatexOk)
    {
        qWarning() << "Unable to write latex to '" << texfile.fileName() << "'";
        emit onFinishedGenerate( nullptr, pdffile);
        return false;
    }   // end if

    qInfo( "Generating PDF from LaTeX...");

    // Need to generate from within the directory.
    RModelIO::PDFGenerator pdfgen(true);
    const bool genOk = pdfgen( texfile.fileName().toStdString(), false);
    if ( !genOk)
    {
        qWarning() << "Failed to generate PDF!";
        emit onFinishedGenerate( nullptr, pdffile);
        return false;
    }   // end if

    // Copy the created pdf to the requested location.
    QFile::copy( _tmpdir.filePath( "report.pdf"), pdffile);

    emit onFinishedGenerate( fm, pdffile);

    return true;
}   // end generate


bool Report::useSVG() const
{
    return !_inkscape.isEmpty() && QFile::exists(_inkscape);
}   // end useSVG


bool Report::writeLatex( QTextStream& os) const
{
    os << "\\documentclass{article}" << endl
       << "\\listfiles" << endl   // Do this to see in the .log file which packages are used
       << "\\usepackage[textwidth=20cm,textheight=25cm]{geometry}" << endl
       << "\\usepackage{graphicx}" << endl
       << "\\usepackage{verbatim}" << endl;

    if ( useSVG())
    {
        os << "\\usepackage{svg}" << endl   // graphicx also included by svg
           << "\\setsvg{inkscape={\"" << _inkscape << "\"}}" << endl
           << "\\usepackage{relsize}" << endl;
    }   // end if

    os << "\\usepackage{float}" << endl
       << "\\usepackage[justification=centering]{caption}" << endl
       << "\\usepackage{media9}" << endl
       << "\\usepackage{amsmath}" << endl
       << "\\usepackage[parfill]{parskip}" << endl
       << "\\usepackage[colorlinks=true,urlcolor=red]{hyperref}" << endl
       << "\\DeclareGraphicsExtensions{.png,.jpg,.pdf,.eps}" << endl;

    // Header
    os << endl
       << "\\usepackage{fancyhdr}" << endl
       << "\\pagestyle{fancy}" << endl
       //<< "\\renewcommand{\\headrulewidth}{0pt}" << endl
       //<< "\\renewcommand{\\footrulewidth}{0pt}" << endl
       << "\\setlength\\headheight{24mm}" << endl
       << "\\addtolength{\\textheight}{-7mm}" << endl
       << "\\rhead{\\includegraphics[width=60mm]{" << _logofile << "}}" << endl
       << "\\lhead{" << endl
       //<< "\\Large " << _headerName << " Report: " << _title << " \\\\" << endl
       << "\\LARGE \\textbf{" << _title << "} \\\\" << endl
       << "\\large \\textbf{Report Date:} " << QDate::currentDate().toString("dd MMMM yyyy") << "\\\\" << endl;
    if ( !_author.isEmpty())
        os << "\\normalsize " << _author << " \\\\" << endl;
    os << "}" << endl;

    // Document
    os << endl
       << "\\begin{document}" << endl
       << "\\pagenumbering{gobble}" << endl
       << "\\thispagestyle{fancy}" << endl << endl;

    bool valid = true;
    try
    {
        sol::function_result result = _content( this, _model);
        if ( result.valid())
        {
            std::string content = result;
            os << content.c_str() << endl;
        }   // end if
    }   // end try
    catch (const sol::error& e)
    {
        qWarning() << "Lua Error!:" << e.what();
        valid = false;
    }   // end catch

    os << "\\end{document}" << endl;
    os.flush();
    return valid;
}   // end writeLaTeX


namespace {
bool writefig( const QString& u3dfile, QTextStream& os, float wmm, float hmm, const QString& caption)
{
    if ( u3dfile.isEmpty())
        return false;

    static int labelID = 0;
    const QString label = QString("label%1").arg(labelID++);

    os.setRealNumberNotation( QTextStream::FixedNotation);
    os.setRealNumberPrecision( 3);

    os << "\\begin{figure}[H]" << endl
       << "\\centering" << endl;

    os << "\\includemedia[" << endl
       << "\tlabel=" << label << "," << endl
       << "\twidth=" << wmm << "mm," << endl
       << "\theight=" << hmm << "mm," << endl
       << "\tadd3Djscript=3Dspintool.js,   % let scene rotate about z-axis" << endl
       << "\tkeepaspectratio," << endl
       << "\tactivate=pageopen," << endl
       << "\tplaybutton=plain,    % plain | fancy (default) | none" << endl
       << "\t3Dbg=1 1 1," << endl
       //<< "\t3Dtoolbar," << endl
       << "\t3Dmenu," << endl
       << "\t3Dviews=views.vws," << endl
       << "\t]{}{" << u3dfile << "}\\\\" << endl;

    /* THESE AREN'T WORKING! (and also aren't formatted well)
    os << "\\mediabutton[3Dgotoview=" << label << ":(Right)]{\\fbox{Right}}" << endl
       << "\\mediabutton[3Dgotoview=" << label << ":(Front)]{\\fbox{Front}}" << endl
       << "\\mediabutton[3Dgotoview=" << label << ":(Left)]{\\fbox{Left}}" << endl;
    */

    if ( !caption.isEmpty())
        os << "\\caption*{" << caption << "}" << endl;

    os << "\\end{figure}" << endl;

    return true;
}   // end writefig


void writeImg( const QString& imgpath, QTextStream& os, const QString& caption)
{
    os << "\\begin{figure}[H]" << endl
       << "\\centering" << endl;
    if ( !caption.isEmpty())
        os << "\\caption*{" << caption << "}" << endl;
    //os << "\\includegraphics[width=\\linewidth]{" << imgpath << "}" << endl;
    os << "\\includegraphics[width=110.00mm]{" << imgpath << "}" << endl;
    os << "\\end{figure}" << endl;
}   // end writeImg


void writeSvg( const QString& imname, QTextStream& os, const QString& caption)
{
    os << "\\begin{figure}[H]" << endl
       << "\\centering" << endl;
    if ( !caption.isEmpty())
        os << "\\caption*{" << caption << "}" << endl;
    os << "\\includesvg[width=98.00mm,pretex=\\relscale{0.8}]{" << imname << "}" << endl;
    os << "\\end{figure}" << endl;
}   // end writeSvg


QString sanit( QString s)
{
    // Note here that the order is very important!
    s.replace("\\", "\\textbackslash");
    s.replace("#", "\\#");
    s.replace("$", "\\$");
    s.replace("%", "\\%");
    s.replace("&", "\\&");
    s.replace("^", "\\textasciicircum");
    s.replace("_", "\\_");
    s.replace("{", "\\{");
    s.replace("}", "\\}");
    s.replace("~", "\\~{}");
    return s;
}   // end sanit

}   // end namespace


std::string Report::makeScanInfo()
{
    QString ostr;
    QTextStream os(&ostr);
    os << " \\textbf{Ethnicity:} " << sanit(Ethnicities::name(_model->maternalEthnicity()));
    if ( _model->maternalEthnicity() != _model->paternalEthnicity())
        os << sanit(" (M) & " + Ethnicities::name(_model->paternalEthnicity()) + " (P)");
    os << " \\\\" << endl;

    // Sex and DOB on one line
    os << " \\textbf{Sex:} " << sanit(FaceTools::toLongSexString( _model->sex()));
    os << " \\textbf{DOB:} " << _model->dateOfBirth().toString("dd MMMM yyyy") << " \\\\" << endl;
    os << " \\textbf{Capture Date:} " << _model->captureDate().toString("dd MMMM yyyy") << " \\\\" << endl;
    const double age = _model->age();
    const int yrs = int(age);
    const int mths = int((age - double(yrs)) * 12);
    os << QString(" \\textbf{Age:} %1 years %2 months \\\\").arg(yrs).arg(mths) << endl;
    if ( !_model->source().isEmpty())
        os << " \\textbf{Source:} " << sanit(_model->source());
    if ( !_model->studyId().isEmpty())
        os << " \\textbf{Study Id:} " << sanit(_model->studyId());
    if ( !_model->source().isEmpty() || !_model->studyId().isEmpty())
        os << " \\\\" << endl;

    return ostr.toStdString();
}   // end makeScanInfo


std::string Report::showNotes()
{
    QString ostr;
    QTextStream os(&ostr);
    FaceAssessment::CPtr ass = _model->currentAssessment();
    os << " \\normalsize{\\textbf{Assessor:} " << sanit(ass->assessor()) << "} \\\\" << endl;
    os << " \\small{" << sanit( ass->hasNotes() ? ass->notes() : "Nothing recorded.") << "} \\\\" << endl;
    return ostr.toStdString();
}   // end showNotes


std::string Report::makeFigure( float wmm, float hmm, const std::string& caption)
{
    QString qcaption(caption.c_str());
    QString ostr;
    QTextStream os( &ostr);
    if ( !writefig( _u3dfile, os, wmm, hmm, qcaption))
        ostr = "";
    return ostr.toStdString();
}   // end makeFigure


std::string Report::makeChart( int mid, size_t d, int footnotemark)
{
    GrowthData::CPtr gd = MCM::metric(mid)->currentGrowthData();
    Metric::Chart* chart = new Metric::Chart( gd, d, _model);

    QtCharts::QChartView cview( chart);
    cview.setRenderHint(QPainter::Antialiasing);

    static const QRectF outRect( 0,0,570,450);
    chart->resize( outRect.size());

    QPaintDevice *img = nullptr;
    QString imgpath;

    if ( useSVG())
    {
        imgpath = _tmpdir.filePath("chart.svg");
        QSvgGenerator* simg = new QSvgGenerator;
        simg->setFileName(imgpath);
        simg->setSize(outRect.toRect().size());
        simg->setViewBox(outRect);
        img = simg;
    }   // end if
    else
    {
        imgpath = _tmpdir.filePath("chart.png");
        QPixmap* pimg = new QPixmap( outRect.toRect().size());
        pimg->fill( Qt::transparent);
        img = pimg;
    }   // end else

    QPainter painter;
    painter.begin( img);
    painter.setRenderHint(QPainter::Antialiasing);
    chart->scene()->render( &painter, outRect);
    painter.end();

    if ( !useSVG() && !static_cast<QPixmap*>(img)->save( imgpath))
    {
        qWarning( "Unable to save PNG!");
        return "";
    }   // end if

    QString qcaption = chart->makeLatexTitleString( footnotemark);
    QString ostr;
    QTextStream os( &ostr);

    if ( useSVG())
    {
        writeSvg( "chart", os, qcaption);
        delete static_cast<QSvgGenerator*>(img);
    }   // end if
    else
    {
        writeImg( imgpath, os, qcaption);
        delete static_cast<QPixmap*>(img);
    }   // end else

    return ostr.toStdString();
}   // end makeChart
