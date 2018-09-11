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

#include <FaceModelXMLFileHandler.h>
#include <FaceShapeLandmarks2DDetector.h>   // FaceTools::Landmarks
#include <FaceTools.h>  // calcFaceCentre
#include <FaceModel.h>
#include <MiscFunctions.h>
#include <AssetImporter.h>
#include <OBJExporter.h>
#include <Orientation.h>    // RFeatures
#include <FileIO.h>         // rlib
#include <QTemporaryDir>
#include <quazip5/JlCompress.h>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <ctime>
using FaceTools::FileIO::FaceModelXMLFileHandler;
using FaceTools::FaceModel;

namespace {

void writeFaceModel( const FaceModel* fm, const std::string& objfname, PTree& rnode)
{
    const std::string relfname = boost::filesystem::path( objfname).filename().string(); // Relative filepath
    rnode.put( "objfilename", relfname);
    rnode.put( "description", fm->description());
    rnode.put( "source", fm->source());
    rnode << fm->orientation();
    if ( fm->centreSet())
        RFeatures::putNamedVertex( rnode, "centre", fm->centre());
    rnode << *fm->landmarks();
    rnode << *fm->paths();
}   // end writeFaceModel


FaceModel* readFaceModel( const PTree& rnode, std::string& objfilename)
{
    FaceModel* fm = new FaceModel;
    objfilename = rnode.get<std::string>( "objfilename");  // Without path info
    fm->setDescription( rnode.get<std::string>( "description"));
    fm->setSource( rnode.get<std::string>( "source"));

    RFeatures::Orientation on;
    rnode >> on;
    fm->setOrientation(on);
    //std::cerr << "Read in orientation " << on << " with DP " << on.norm().dot( on.up()) << std::endl;

    cv::Vec3f c(0,0,0); // Get the centre if in the file
    bool gotcentre = RFeatures::getNamedVertex( rnode, "centre", c);

    // Get the landmarks
    FaceTools::LandmarkSet::Ptr lmks = fm->landmarks();
    rnode >> *lmks;

    // Calculate the centre from the landmarks if unable to retrieve from the file
    if (!gotcentre && FaceTools::hasReqLandmarks( lmks))
    {
        using namespace FaceTools::Landmarks;
        c = FaceTools::calcFaceCentre( lmks->pos(L_EYE_CENTRE), lmks->pos(R_EYE_CENTRE), lmks->pos(NASAL_TIP));
        gotcentre = true;
    }   // end if

    // Set the centre if read or calculated, otherwise it'll just be the middle of the largest component.
    if ( gotcentre)
    {
        std::cerr << "Centre set to " << c << std::endl;
        fm->setCentre(c);
    }   // end if
    else
        std::cerr << "Centre set as centre of largest model component" << std::endl;

    FaceTools::PathSet::Ptr paths = fm->paths();
    rnode >> *paths;

    return fm;
}   // end readFaceModel

}   // end namespace


// public
FaceModel* FaceModelXMLFileHandler::read( const QString& sfname)
{
    _err = "";
    FaceModel* fm = nullptr;

    try
    {
        QTemporaryDir dir;
        if ( !dir.isValid())
        {
            _err = "Unable to open temporary directory for reading from!";
            return nullptr;
        }   // end if

        QStringList fnames = JlCompress::extractDir( sfname, dir.path());
        if ( fnames.isEmpty())
        {
            _err = "Unable to extract files from archive!";
            return nullptr;
        }   // end if

        std::string xmlfile = boost::filesystem::path(sfname.toStdString()).filename().replace_extension( "xml").string();   // 13040011.xml
        xmlfile = dir.filePath( xmlfile.c_str()).toStdString();
        if ( !boost::filesystem::exists( xmlfile) || !boost::filesystem::is_regular_file( xmlfile))
        {
            _err = "Cannot find XML meta-data file in archive!";
            return nullptr;
        }   // end if

        std::ifstream ifs;
        ifs.open( xmlfile);
        if ( !ifs.is_open())
        {
            std::ostringstream serr;
            serr << "Unable to open \"" << xmlfile << "\" for reading!";
            _err = serr.str().c_str();
            return nullptr;
        }   // end if

        PTree tree;
        boost::property_tree::read_xml( ifs, tree);

        const PTree& faces = tree.get_child( "faces");
        const std::string fversion = faces.get<std::string>( "<xmlattr>.version");
        if ( fversion != XML_VERSION)
        {
            std::ostringstream serr;
            serr << "Invalid XML file version (got " << fversion << " but need " << XML_VERSION << ")";
            _err = serr.str().c_str();
            return nullptr;
        }   // end if

        const std::string filedesc = faces.get<std::string>( "description");
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelXMLFileHandler::read: Reading file with description \""
                  << filedesc << "\"" << std::endl;

        const PTree& records = faces.get_child( "FaceModels");
        const int nrecs = records.get<int>( "<xmlattr>.count");
        if ( nrecs <= 0)
        {
            _err = "No FaceModel objects recorded in file!";
            return nullptr;
        }   // end if

        if ( nrecs > 1)
        {
            std::cerr << "[WARNING] FaceTools::FileIO::FaceModelXMLFileHandler::read: Multiple FaceModel records not supported!"
                      << " Ignoring all records after the first." << std::endl;
        }   // end if

        std::string objfilename;
        for ( const PTree::value_type& rnode : records)
        {
            if ( rnode.first == "FaceModel")
            {
                fm = readFaceModel( rnode.second, objfilename);
                break;
            }   // end if
        }   // end for

        if ( !fm)
        {
            _err = "Unable to read in FaceModel from file record!";
            return nullptr;
        }   // end if

        const std::string fext = rlib::getExtension( objfilename);
        if ( fext != "obj")
        {
            _err = "FaceModel has its model saved in an unsupported format!";
            delete fm;
            return nullptr;
        }   // end if

        RModelIO::AssetImporter importer(true,true);
        importer.enableFormat(fext);
        const std::string objfile = dir.filePath( objfilename.c_str()).toStdString();
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelXMLFileHandler::read: Loading model from \"" << objfile << "\"" << std::endl;
        RFeatures::ObjModel::Ptr model = importer.load( objfile);   // Doesn't merge materials or clean!
        if ( !model)
        {
            std::ostringstream serr;
            serr << "Failed to load object from \"" << objfile << "\"";
            _err = serr.str().c_str();
            delete fm;
            return nullptr;
        }   // end if

        RFeatures::ObjModelInfo::Ptr minfo = RFeatures::ObjModelInfo::create(model);
        if ( !minfo)
        {
            std::ostringstream serr;
            serr << "Failed to clean object loaded from \"" << objfile << "\"";
            _err = serr.str().c_str();
            delete fm;
            return nullptr;
        }   // end if

        const bool uokay = fm->update( minfo);
        assert(uokay);
    }   // end try
    catch ( const boost::property_tree::ptree_bad_path& e)
    {
        _err = "XML bad path error encountered reading in stream data!";
        std::cerr << e.what() << std::endl;
    }   // end catch
    catch ( const boost::property_tree::xml_parser_error& e)
    {
        _err = "XML parse error encountered reading in stream data!";
        std::cerr << e.what() << std::endl;
    }   // end catch
    catch ( const std::exception& e)
    {
        _err = "Unable to read in stream data!";
        std::cerr << e.what() << std::endl;
    }   // end catch

    if ( !_err.isEmpty() && fm)
    {
        delete fm;
        fm = nullptr;
    }   // end if

    return fm;
}   // end read



// public
bool FaceModelXMLFileHandler::write( const FaceModel* fm, const QString& sfname)
{
    assert(fm);
    _err = "";

    try
    {
        std::string fname = sfname.toStdString();   // /home/rich/Desktop/13040011.3df
        QTemporaryDir dir( boost::filesystem::path(fname).parent_path().string().c_str());
        if ( !dir.isValid())
        {
            _err = "Unable to create temporary directory for writing to!";
            return false;
        }   // end if

        std::string xmlfile = boost::filesystem::path(fname).filename().replace_extension( "xml").string();   // 13040011.xml
        xmlfile = dir.filePath( xmlfile.c_str()).toStdString();

        std::ofstream ofs;
        ofs.open( xmlfile);
        if ( !ofs.is_open())
        {
            _err = ("Unable to open \"" + xmlfile + "\" for writing!").c_str();
            return false;
        }   // end if

        // Write out the model geometry itself into .obj format.
        RModelIO::OBJExporter exporter;
        const std::string modfile = boost::filesystem::path(xmlfile).replace_extension( "obj").string();
        const RFeatures::ObjModel* model = fm->info()->cmodel();
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelXMLFileHandler::write: Exporting model to " << modfile << std::endl;
        if ( !exporter.save( model, modfile))
        {
            _err = exporter.err().c_str();
            std::cerr << "[WARNING] FaceTools::FileIO::FaceModelXMLFileHandler::write: Failed! " << _err.toStdString() << std::endl;
            return false;
        }   // end if

        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelXMLFileHandler::write: Exported object to " << modfile << std::endl;

        PTree tree;
        PTree& topNode = tree.put( "faces","");
        topNode.put( "<xmlattr>.version", XML_VERSION);
        std::ostringstream desc;
        desc << XML_FILE_DESCRIPTION << ";" << time(nullptr);
        topNode.put( "description", desc.str());

        PTree& records = topNode.put( "FaceModels","");    // Only a single record
        records.put( "<xmlattr>.count", 1);
        PTree& rnode = records.add("FaceModel","");
        writeFaceModel( fm, modfile, rnode);

        boost::property_tree::write_xml( ofs, tree);
        ofs.close();

        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelXMLFileHandler::write: Exported meta-data to " << xmlfile << std::endl;

        // Finally, zip up the contents of the directory into sfname.
        if ( !JlCompress::compressDir( sfname, dir.path(), true/*recursively pack subdirs*/))
            _err = "Unable to compress saved data into archive format!";
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << "[EXCEPTION] FaceTools::FileIO::FaceModelXMLFileHandler::write: Failed to write to "
            << sfname.toStdString() << std::endl;
        _err = e.what();
    }   // end catch

    return _err.isEmpty();
}   // end write
