#include "./server.hpp"

#include "./task_echo.hpp"

int main( int argc, char* argv[] )
{
    Server< TaskEcho >( argv[1] ).run();
}

