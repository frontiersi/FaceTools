#include <FaceTools.h>
#include <FaceModelManager.h>
#include <FaceModelOBJFileHandler.h>
#include <FaceModelXMLFileHandler.h>
#include <OffscreenModelViewer.h>
using FaceTools::FM;


int main( int argc, char *argv[])
{
    if ( argc == 1)
    {
        std::cerr << "Pass in model filename" << std::endl;
        return EXIT_FAILURE;
    }   // end if

    using namespace FaceTools::FileIO;
    FMM::add( new FaceModelXMLFileHandler);
    FMM::add( new FaceModelOBJFileHandler);
    FM* fm = FMM::read( argv[1]);
    if ( !fm)
    {
        std::cerr << "Unable to read in FaceModel from " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }   // end if

    const double d = 600;
    const cv::Size isz(512,512);

    //RVTK::OffscreenModelViewer vwr( isz, d);
    //vwr.setModel( fm->info()->cmodel());

    int c = 0;
    char key = 27;  // Esc
    cv::Mat img;
    do
    {
        //img = vwr.snapshot();
        img = FaceTools::makeThumbnail( fm, isz, d);
        RFeatures::showImage( img, "Snapshot", false);
        std::cerr << std::setw(3) << std::right << (c++) << ") ESC to exit; any other key to take another snapshot." << std::endl;
        key = cv::waitKey();
    } while ( key != 27);

    return EXIT_SUCCESS;
}   // end main
