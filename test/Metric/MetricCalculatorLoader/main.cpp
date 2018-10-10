#include <MetricCalculatorLoader.h>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>

int main( int argc, char **argv)
{
    if ( argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " datafile" << std::endl;
        return EXIT_FAILURE;
    }   // end if

    using namespace FaceTools::Metric;
    MetricCalculatorLoader loader( argv[1]);
    MC* mc = loader.load();

    return EXIT_SUCCESS;
}   // end main
