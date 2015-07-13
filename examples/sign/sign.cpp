#include "./server.hpp"

int main( int argc, char* argv[] )
{
    SignServer< Key >( argv[1] ).run();
}

