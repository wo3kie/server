#include "../core/server.hpp"

#include "./echo_server.hpp"
#include "./echo_task.hpp"

int main( int argc, char* argv[] )
{
    EchoServer( argv[ 1 ] ).run();
}

