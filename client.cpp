/*
 * Lukasz Czerwinski
 */

#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>


namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;

struct Task
{
    Task(
        asio::io_service & ioService,
        ip::tcp::socket & socket
    )
        : m_ioService( ioService )
        , m_socket( socket )
    {
    }

    void run()
    {
        auto const threadBody = boost::bind(
            & asio::io_service::run,
            & m_ioService
        );

        boost::thread thread( threadBody );

        runImpl();

        thread.join();
    }

protected:

    virtual void runImpl() = 0;

protected:

    asio::io_service & m_ioService;
    ip::tcp::socket & m_socket;

};

struct Writer : Task
{
    Writer(
        asio::io_service & ioService,
        ip::tcp::socket & socket
    )
        : Task( ioService, socket )
    {
    }

protected:

    void runImpl() override
    {
        auto const onRequestWritten = boost::bind(
            & Writer::onRequestWritten,
            this,
            placeholders::error
        );

        while( std::cin.getline( m_request, m_maxLength + 1 ) )
        {
            asio::async_write(
                m_socket,
                asio::buffer( m_request, strlen( m_request ) ),
                onRequestWritten
            );
        }
    }

private:

    void onRequestWritten(
        sys::error_code const & errorCode
    )
    {
        if( errorCode )
        {
            std::cerr << "Request Write Error: " << errorCode.message() << std::endl;
        }
    }

private:

    enum { m_maxLength = 1024 };
    char m_request[ m_maxLength ];
};

struct Reader : Task
{
    Reader(
        asio::io_service & ioService,
        ip::tcp::socket & socket
    )
        : Task( ioService, socket )
    {
    }

protected:

    void runImpl() override
    {
        auto const onResponseRead = boost::bind(
            & Reader::onResponseRead,
            this,
            placeholders::error,
            placeholders::bytes_transferred
        );

        m_socket.async_read_some(
            asio::buffer( m_response, m_maxLength ),
            onResponseRead
        );
    }

private:

    void onResponseRead(
        sys::error_code const & errorCode,
        size_t const bytesTransferred
    )
    {
        if( errorCode )
        {
            std::cerr << "Response Read Error: " << errorCode.message() << std::endl;
        }
        else
        {
            std::cout << "> ";
            std::cout.write( m_response, bytesTransferred );
            std::cout << std::endl;

            m_ioService.post(
                boost::bind( & Reader::runImpl, this )
            );
        }
    }

private:

    enum { m_maxLength = 1024 };
    char m_response[ m_maxLength ];
};

struct Client
{
    Client(
        asio::io_service & ioService,
        std::string const & host,
        std::string const & port
    )
        : m_ioService( ioService )
        , m_socket( m_ioService )
        , m_reader( m_ioService, m_socket )
        , m_writer( m_ioService, m_socket )
    {
        ip::tcp::resolver::query query( host, port ); 

        auto const onResolved = boost::bind(
            & Client::onResolved,
            this,
            placeholders::error,
            placeholders::iterator
        );

        ip::tcp::resolver resolver( m_ioService );
        resolver.async_resolve(
            query,
            onResolved
        );

        boost::thread( boost::bind( & Writer::run, m_writer ) ).join();
    }

private:

    void onResolved(
        sys::error_code const & errorCode,
        ip::tcp::resolver::iterator endpointIterator 
    )
    {
        if( errorCode )
        {
            std::cerr << "Resolution Error: " << errorCode.message() << std::endl;
        }
        else
        {
            auto const onConnected = boost::bind(
                & Client::onConnected,
                this,
                placeholders::error
            );

            asio::async_connect(
                m_socket,
                endpointIterator,
                onConnected 
            );
        }
    }

    void onConnected(
        sys::error_code const & errorCode 
    )
    {
        if( errorCode )
        {
            std::cerr << "Connection Error: " << errorCode.message() << std::endl;
        }
        else
        {
            m_reader.run();
        }
    }

    asio::io_service & m_ioService;
    ip::tcp::socket m_socket;

    Reader m_reader;
    Writer m_writer;
};

int main( int argc, char* argv[] )
{
    boost::asio::io_service ioService;

    const auto host = "127.0.0.1";
    const auto port = argv[ 1 ];

    Client c( ioService, host, port );

    return 0;
}

