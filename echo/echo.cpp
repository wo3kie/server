#include "../core/server.hpp"

#include "./server.hpp"
#include "./task.hpp"

int main( int argc, char* argv[] )
{
    EchoServer( argv[ 1 ] ).run();
}

