#include "./server.hpp"

#include "./server_chat.hpp"

int main( int argc, char* argv[] )
{
    Server< TaskChat >( argv[1] ).run();
}

