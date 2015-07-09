#include "./echo_server.hpp"

#include "./echo_task.hpp"

int main( int argc, char* argv[] )
{
    EchoServer< EchoTask >( argv[1] ).run();
}

