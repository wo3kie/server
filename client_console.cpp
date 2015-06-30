#include "./client.hpp"

#include "./client_console.hpp"

int main( int argc, char* argv[] )
{
    boost::asio::io_service ioService;

    const auto host = "127.0.0.1";
    const auto port = argv[ 1 ];

    Client< Reader, Writer > c( ioService, host, port );

    return 0;
}

