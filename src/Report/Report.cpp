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

#include <Report.h>
#include <PDFGenerator.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <Ethnicities.h>
#include <PhenotypeManager.h>
#include <Chart.h>
#include <MathUtil.h>   // rlib
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


namespace {
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


Report::Report( QTemporaryDir& tdir) : _tmpdir(tdir), _os(nullptr), _model(nullptr)
{
    // Create some standardised Lua scripting functions for inserting report elements.
    addCustomLuaFn( "addScanInfo", [this](){ this->_addLatexScanInfo();});
    addCustomLuaFn( "addNotes", [this](){ this->_addLatexNotes();});
    addCustomLuaFn( "addPhenotypicVariationsList", [this](){ this->_addLatexPhenotypicVariationsList();});
    addCustomLuaFn( "addStartColumn", [this](){ this->_addLatexStartMinipage();});
    addCustomLuaFn( "addEndColumn", [this](){ this->_addLatexEndMinipage();});
    addCustomLuaFn( "addLineBreak", [this](){ this->_addLatexLineBreak();});

    _lua.set_function( "addFigure",
                       [this]( float widthMM, float heightMM, const std::string& caption)
                        { this->_addLatexFigure( widthMM, heightMM, caption);});

    _lua.set_function( "addGrowthCurvesChart",
                       [this]( int mid, size_t d, int footnotemark)
                        { this->_addLatexGrowthCurvesChart( mid, d, footnotemark);});

    _lua.set_function( "addCustomLatex", [this]( const std::string& s) { this->addCustomLatex( QString::fromStdString(s));});
    _lua.set_function( "addFootnoteSources", [this]( const sol::table &mids){ this->_addFootnoteSources(mids);});

    _lua.set_function( "metric", MCM::metric);
    _lua.set_function( "metricSource", [this](int mid){ return this->_metricCurrentSource(mid).toStdString();});
    _lua.set_function( "footnoteIndices", [this]( const sol::table &mids){ return this->_footnoteIndices(mids);});
    _lua.set_function( "round", []( double v, size_t nd){ return rlib::round(v,nd);});

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


Report::~Report() {}


void Report::addCustomLuaFn( const QString& fnName, const std::function<void()>& fn)
{
    _lua.set_function( fnName.toStdString(), fn);
}   // end addCustomLuaFn


void Report::addCustomLatex( const QString &s)
{
    assert(_os);
    QTextStream& os = *_os;
    os << s << endl;
}   // end addCustomLatex


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

#ifdef NDEBUG
    // Reject the TEST report if this is the release build
    if ( report->name().toUpper() == "TEST")
    {
        std::cerr << "[INFO] FaceTools::Report::Report::load: Skipping test report for release version." << std::endl;
        return nullptr;
    }   // end if
#endif

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

    if ( sol::optional<sol::function> v = table["addContent"])
        report->_addContent = v.value();
    else
    {
        qWarning() << "Missing 'addContent' function!";
        return nullptr;
    }   // end else

    return report;
}   // end load


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

    _os = new QTextStream( &texfile);
    const bool writtenLatexOk = _writeLatex( *_os);
    texfile.close();
    delete _os;

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


bool Report::_useSVG() const
{
    return !_inkscape.isEmpty() && QFile::exists(_inkscape);
}   // end _useSVG


bool Report::_writeLatex( QTextStream& os) const
{
    os << "\\documentclass{article}" << endl
       << "\\listfiles" << endl   // Do this to see in the .log file which packages are used
       << "\\usepackage[textwidth=20cm,textheight=25cm]{geometry}" << endl
       << "\\usepackage{graphicx}" << endl
       << "\\usepackage{verbatim}" << endl
       << "\\usepackage{xcolor}" << endl;

    if ( _useSVG())
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
       << "\\Large \\textbf{" << sanit(_title) << "} \\\\" << endl;

    os << "\\vspace{2mm} \\normalsize" << endl; // Small gap below title

    // Source and Study ID if present
    bool hasSource = !_model->source().isEmpty();
    bool hasStudyId = !_model->studyId().isEmpty();
    if ( hasSource)
        os << "\\textbf{Source:} " << sanit(_model->source());
    if ( hasStudyId)
    {
        if ( hasSource)
            os << "\\hspace{3mm}";
        os << "\\textbf{Study Id:} " << sanit(_model->studyId());
    }   // end if
    if ( hasSource || hasStudyId)
        os << " \\\\" << endl;

    os << "\\textbf{Reporting Date:} " << QDate::currentDate().toString("dd MMMM yyyy") << "\\\\" << endl;

    os << "}" << endl;

    // Document
    os << endl
       << "\\begin{document}" << endl
       << "\\pagenumbering{gobble}" << endl
       << "\\thispagestyle{fancy}" << endl << endl;

    bool valid = true;
    try
    {
        _addContent( _model);   // Call out to Lua function to add the report elements
    }   // end try
    catch (const sol::error& e)
    {
        qWarning() << "Lua Error: " << e.what();
        valid = false;
    }   // end catch

    os << "\\end{document}" << endl;
    os.flush();
    return valid;
}   // end _writeLaTeX


namespace {
bool writefig( const QString& u3dfile, QTextStream& os, float wmm, float hmm, const QString& caption)
{
    if ( u3dfile.isEmpty())
        return false;

    static int labelID = 0;
    const QString label = QString("label%1").arg(labelID++);

    os.setRealNumberNotation( QTextStream::FixedNotation);
    os.setRealNumberPrecision( 3);

    os << R"(\begin{figure}[H]
            \centering
            \includemedia[
            label=)" << label << "," << endl
       << "width=" << wmm << "mm," << endl
       << "height=" << hmm << "mm," << endl
       << R"(add3Djscript=3Dspintool.js,   % let scene rotate about z-axis
             keepaspectratio,
             activate=pageopen,
             playbutton=plain,    % plain | fancy (default) | none
             3Dbg=1 1 1,
             3Dmenu,
             3Dviews=views.vws,
             ]{}{)" << u3dfile << R"(}\\)" << endl;
       //<< "3Dtoolbar," << endl

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
}   // end namespace


void Report::_addLatexFigure( float wmm, float hmm, const std::string& caption)
{
    assert(_os);
    writefig( _u3dfile, *_os, wmm, hmm, QString( caption.c_str()));
}   // end _addLatexFigure


void Report::_addLatexGrowthCurvesChart( int mid, size_t d, int footnotemark)
{
    MC::CPtr mc = MCM::metric(mid);
    assert(mc);
    if ( !mc)
        return;

    // Ensure the chart name is unique (client may want more than one in a report).
    static int chartid = 0;
    const QString imname = QString("chart_%1").arg(abs(chartid));   // abs (LOL)
    chartid++;

    GrowthData::CPtr gd = mc->currentGrowthData();
    Metric::Chart* chart = new Metric::Chart( gd, d, _model);

    QtCharts::QChartView cview( chart);
    cview.setRenderHint(QPainter::Antialiasing);

    static const QRectF outRect( 0,0,570,450);
    chart->resize( outRect.size());

    QPaintDevice *img = nullptr;
    QString imgpath;

    if ( _useSVG())
    {
        imgpath = _tmpdir.filePath(QString("%1.svg").arg(imname));
        QSvgGenerator* simg = new QSvgGenerator;
        simg->setFileName(imgpath);
        simg->setSize(outRect.toRect().size());
        simg->setViewBox(outRect);
        img = simg;
    }   // end if
    else
    {
        imgpath = _tmpdir.filePath(QString("%1.png").arg(imname));
        QPixmap* pimg = new QPixmap( outRect.toRect().size());
        pimg->fill( Qt::transparent);
        img = pimg;
    }   // end else

    QPainter painter;
    painter.begin( img);
    painter.setRenderHint(QPainter::Antialiasing);
    chart->scene()->render( &painter, outRect);
    painter.end();

    if ( !_useSVG() && !static_cast<QPixmap*>(img)->save( imgpath))
    {
        qWarning( "Unable to save PNG!");
        return;
    }   // end if

    QString qcaption = chart->makeLatexTitleString( footnotemark);
    assert(_os);
    QTextStream& os = *_os;
    os << R"(\begin{figure}[H]
             \centering)" << endl;
    if ( !qcaption.isEmpty())
        os << "\\caption*{" << qcaption << "}" << endl;

    if ( _useSVG())
    {
        os << R"(\includesvg[width=98.00mm,pretex=\relscale{0.8}]{)" << imname << "}" << endl;
        delete static_cast<QSvgGenerator*>(img);
    }   // end if
    else
    {
        //os << "\\includegraphics[width=\\linewidth]{" << imgpath << "}" << endl;
        os << "\\includegraphics[width=110.00mm]{" << imgpath << "}" << endl;
        delete static_cast<QPixmap*>(img);
    }   // end else

    os << "\\end{figure}" << endl;
}   // end _addLatexGrowthCurvesChart


void Report::_addLatexScanInfo()
{
    assert(_os);
    QTextStream& os = *_os;
    os << "\\textbf{Image Capture Date:} " << _model->captureDate().toString("dd MMMM yyyy") << " \\\\" << endl;

    os << "\\textbf{Ethnicity:} " << sanit(Ethnicities::name(_model->maternalEthnicity()));
    if ( _model->maternalEthnicity() != _model->paternalEthnicity())
        os << sanit(" (M) & " + Ethnicities::name(_model->paternalEthnicity()) + " (P)");
    os << " \\\\" << endl;

    // Sex and DOB on one line
    os << "\\textbf{Sex:} " << sanit(FaceTools::toLongSexString( _model->sex()));
    os << "\\hspace{3mm} \\textbf{DOB:} " << _model->dateOfBirth().toString("dd MMMM yyyy") << " \\\\" << endl;
    const double age = _model->age();
    const int yrs = int(age);
    const int mths = int((age - double(yrs)) * 12);
    os << QString("\\textbf{Age:} %1 years %2 months \\\\").arg(yrs).arg(mths) << endl;
}   // end _addLatexScanInfo


void Report::_addLatexNotes()
{
    assert(_os);
    QTextStream& os = *_os;

    FaceAssessment::CPtr ass = _model->currentAssessment();
    os << R"( \normalsize{\textbf{Assessor:} )" << sanit(ass->assessor()) << R"(} \\)" << endl;
    os << R"( \small{)" << sanit( ass->hasNotes() ? ass->notes() : "Nothing recorded.") << R"(} \\)" << endl;
}   // end _addLatexNotes


void Report::_addLatexPhenotypicVariationsList()
{
    assert(_os);
    QTextStream& os = *_os;

    const IntSet pids = PhenotypeManager::discover(_model, -1); // Use the current assessment id
    if ( pids.empty())
        os << "No atypical phenotypic variations identified." << endl;
    else
    {
        os << "\\textbf{Identified atypical phenotypic variations:}" << endl
           << "\\begin{itemize}" << endl;
        for ( int pid : pids)
            os << "\t\\item " << PhenotypeManager::phenotype(pid)->name() << endl;
        os << "\\end{itemize}" << endl;
    }   // end else
}   // end _addLatexPhenotypicVariationsList


void Report::_addLatexStartMinipage()
{
    assert(_os);
    QTextStream& os = *_os;
    os << R"(\begin{minipage}[t]{.5\linewidth})" << endl;
}   // end _addLatexStartMinipage


void Report::_addLatexEndMinipage()
{
    assert(_os);
    QTextStream& os = *_os;
    os << R"(\end{minipage})" << endl;
}   // end _addLatexEndMinipage


void Report::_addLatexLineBreak()
{
    assert(_os);
    QTextStream& os = *_os;
    os << R"(\hfill \break)" << endl;
}   // end _addLatexLineBreak


QString Report::_metricCurrentSource( int mid) const
{
    GrowthData::CPtr gd;
    if ( MCM::metric(mid))
        gd = MCM::metric(mid)->currentGrowthData();
    if ( !gd)
        return "";
    QString src = gd->source();
    if ( !gd->note().isEmpty())
        src += " " + gd->note();
    return src;
}   // end _metricCurrentSource


std::unordered_map<int, int> Report::_footnoteIndices( const sol::table& mids) const
{
    QMap<QString, int> refs;
    std::unordered_map<int, int> idxs;
    int idx = 1;
    for ( size_t i = 0; i < mids.size(); ++i)
    {
        const int mid = mids[i];
        const QString src = _metricCurrentSource(mid);
        if ( src.isEmpty())
            continue;

        if ( refs.count(src) > 0)
            idxs[mid] = refs.value(src);
        else
        {
            refs[src] = mid;
            idxs[mid] = idx++;
        }   // end else
    }   // end for
    return idxs;
}   // end _footnoteIndices


void Report::_addFootnoteSources( const sol::table& mids)
{
    assert(_os);
    QTextStream& os = *_os;

    QSet<QString> refs;
    int idx = 1;
    for ( size_t i = 0; i < mids.size(); ++i)
    {
        const int mid = mids[i];
        const QString src = _metricCurrentSource(mid);
        if ( !src.isEmpty() && !refs.contains(src))
        {
            refs.insert(src);
            os << "\\footnotetext[" << idx << "]{" << src << "}" << endl;
            idx++;
        }   // end if
    }   // end for
}   // end _addFootnoteSources
