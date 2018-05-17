/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
#include <FaceModel.h>
#include <MiscFunctions.h>
#include <AssetImporter.h>
#include <OBJExporter.h>
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
    rnode.put( "objfilename", boost::filesystem::path( objfname).filename());   // Without path info
    rnode.put( "description", fm->description());
    rnode.put( "source", fm->source());
    rnode << fm->orientation();
    rnode << fm->landmarks();
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
    rnode >> fm->landmarks();

    return fm;
}   // end readFaceModel

}   // end namespace


// public
FaceModel* FaceModelXMLFileHandler::read( const QString& sfname)
{
    _err = "";
    FaceModel* fm = NULL;
    const std::string fname = sfname.toStdString();

    try
    {
        std::ifstream ifs;
        ifs.open( fname);
        if ( !ifs.is_open())
        {
            std::ostringstream serr;
            serr << "Unable to open \"" << fname << "\" for reading!";
            _err = serr.str().c_str();
            return NULL;
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
            return NULL;
        }   // end if

        const std::string filedesc = faces.get<std::string>( "description");
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelXMLFileHandler::read: Reading file with description \""
                  << filedesc << "\"" << std::endl;

        const PTree& records = faces.get_child( "FaceModels");
        const int nrecs = records.get<int>( "<xmlattr>.count");
        if ( nrecs <= 0)
        {
            _err = "No FaceModel objects recorded in file!";
            return NULL;
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
            return NULL;
        }   // end if

        const std::string fext = FaceTools::getExtension( objfilename);
        if ( fext != "obj")
        {
            _err = "FaceModel has its model saved in an unsupported format!";
            delete fm;
            return NULL;
        }   // end if

        RModelIO::AssetImporter importer(true,true);
        importer.enableFormat(fext);
        const std::string modelfile = (boost::filesystem::path(fname).parent_path() / objfilename).string();
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelXMLFileHandler::read: Loading model from \"" << modelfile << "\"" << std::endl;
        RFeatures::ObjModel::Ptr model = importer.load( modelfile);   // Doesn't merge materials or clean!
        if ( !model)
        {
            std::ostringstream serr;
            serr << "Failed to load object from \"" << modelfile << "\"";
            _err = serr.str().c_str();
            delete fm;
            return NULL;
        }   // end if

        if (!fm->updateData( model))
        {
            std::ostringstream serr;
            serr << "Failed to clean object loaded from \"" << modelfile << "\"";
            _err = serr.str().c_str();
            delete fm;
            return NULL;
        }   // end if
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
        fm = NULL;
    }   // end if

    return fm;
}   // end read



// public
bool FaceModelXMLFileHandler::write( const FaceModel* fm, const QString& sfname)
{
    assert(fm);
    _err = "";
    const std::string fname = sfname.toStdString();

    try
    {
        std::ofstream ofs;
        ofs.open( fname);
        if ( !ofs.is_open())
        {
            _err = ("Unable to open \"" + fname + "\" for writing!").c_str();
            return false;
        }   // end if

        // Write out the model geometry itself into .obj format.
        RModelIO::OBJExporter exporter;
        const std::string modfile = boost::filesystem::path(fname).replace_extension( "obj").string();
        const RFeatures::ObjModel* model = fm->cmodel();
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelXMLFileHandler::write: Exporting model to " << modfile << std::endl;
        if ( !exporter.save( model, modfile))
        {
            _err = exporter.err().c_str();
            std::cerr << "[WARNING] FaceTools::FileIO::FaceModelXMLFileHandler::write: Failed! " << _err.toStdString() << std::endl;
            return false;
        }   // end if

        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelXMLFileHandler::write: Exported model to " << modfile << std::endl;
        std::cerr << "[STATUS] FaceTools::FileIO::FaceModelXMLFileHandler::write: Writing meta-data out to " << fname << std::endl;

        PTree tree;
        PTree& topNode = tree.put( "faces","");
        topNode.put( "<xmlattr>.version", XML_VERSION);
        std::ostringstream desc;
        desc << XML_FILE_DESCRIPTION << ";" << time(NULL);
        topNode.put( "description", desc.str());

        PTree& records = topNode.put( "FaceModels","");    // Only a single record
        records.put( "<xmlattr>.count", 1);
        PTree& rnode = records.add("FaceModel","");
        writeFaceModel( fm, modfile, rnode);

        boost::property_tree::write_xml( ofs, tree);
        ofs.close();
    }   // end try
    catch ( const std::exception& e)
    {
        std::cerr << "[EXCEPTION] FaceTools::FileIO::FaceModelXMLFileHandler::write: Failed to write to " << fname << std::endl;
        _err = e.what();
    }   // end catch

    return _err.isEmpty();
}   // end write
