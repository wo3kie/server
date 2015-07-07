#include "./server.hpp"

#include "./server_sign.hpp"

int main( int argc, char* argv[] )
{
    Server< TaskSign, Key >( argv[1] ).run();
}

