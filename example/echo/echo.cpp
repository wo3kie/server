#include "./server.hpp"

int main( int argc, char* argv[] )
{
    EchoServer( argv[ 1 ] ).run();
}

