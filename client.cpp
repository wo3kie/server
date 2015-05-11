/*
 * Lukasz Czerwinski
 */

#include <iostream>
#include <string>

#include <boost/asio.hpp>

#include <boost/bind.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;

struct Client
{
    Client(
        boost::asio::io_service & ioService,
        std::string const & host,
        std::string const & port
    )
        : m_socket( ioService )
        , m_resolver( ioService )
    {
        ip::tcp::resolver::query query( host, port ); 

        auto const onResolve = boost::bind(
            & Client::onResolve,
            this,
            placeholders::error,
            placeholders::iterator
        );

        m_resolver.async_resolve(
            query,
            onResolve
        );

        ioService.run();
    }

    void onResolve(
        sys::error_code const & errorCode,
        ip::tcp::resolver::iterator endpointIterator 
    )
    {
        if( errorCode )
        {
            std::cerr << "Error: " << errorCode.message() << std::endl;
        }
        else
        {
            auto const onConnect = boost::bind(
                & Client::onConnect,
                this,
                placeholders::error
            );

            asio::async_connect(
                m_socket,
                endpointIterator,
                onConnect 
            );
        }
    }

    void onConnect(
        sys::error_code const & errorCode 
    )
    {
        if( errorCode )
        {
            std::cerr << "Error: " << errorCode.message() << std::endl;
        }
        else
        {
            prepareAndSendRequest();
        }
    }

    void prepareAndSendRequest()
    {
        auto const onWrite = boost::bind(
            & Client::onWriteRequest,
            this,
            placeholders::error
        );

        std::cout << "< ";
        std::cin.getline( m_request, m_maxLength );

        asio::async_write(
            m_socket,
            asio::buffer( m_request, strlen( m_request ) ),
            onWrite
        );
    }

    void onWriteRequest(
        sys::error_code const & errorCode 
    )
    {
        if( errorCode )
        {
            std::cerr << "Error: " << errorCode.message() << std::endl;
        }
        else
        {
            auto const onReadResponse = boost::bind(
                & Client::onReadResponse,
                this,
                placeholders::error,
                placeholders::bytes_transferred
            );

            m_socket.async_read_some(
                asio::buffer( m_response, m_maxLength ),
                onReadResponse
            );
        }
    }

    void onReadResponse(
        sys::error_code const & errorCode,
        size_t const bytesTransferred
    )
    {
        if( errorCode )
        {
            std::cerr << "Error: " << errorCode.message() << std::endl;
        }
        else
        {
            std::cout << "> ";
            std::cout.write( m_response, bytesTransferred );
            std::cout << std::endl;

            prepareAndSendRequest();
        }
    }

    ip::tcp::socket m_socket;
    ip::tcp::resolver m_resolver;

    enum { m_maxLength = 1024 };

    char m_request[ m_maxLength ];
    char m_response[ m_maxLength ];
};

int main( int argc, char* argv[] )
{
    boost::asio::io_service ioService;

    const auto host = argv[ 1 ];
    const auto port = argv[ 2 ];

    Client c( ioService, host, port );

    return 0;
}

