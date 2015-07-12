#include "./server.hpp"

int main( int argc, char* argv[] )
{
    ChatServer( argv[ 1 ] ).run();
}

