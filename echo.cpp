#include "./server.hpp"

#include "./server_echo.hpp"

int main( int argc, char* argv[] )
{
    Server< TaskEcho >( argv[1] ).run();
}

