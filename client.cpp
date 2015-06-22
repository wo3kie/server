/*
 * Lukasz Czerwinski
 */

#include <iostream>
#include <string>

#include <boost/asio.hpp>

#ifdef SERVER_SSL
#include <boost/asio/ssl.hpp>
#endif

#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;

#ifdef SERVER_SSL
namespace ssl = asio::ssl;
#endif

struct Task
{
    Task(
        asio::io_service & ioService,

#ifdef SERVER_SSL
        ssl::stream< ip::tcp::socket > & socket    
#else
        ip::tcp::socket & socket
#endif

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

#ifdef SERVER_SSL
    ssl::stream< ip::tcp::socket > & m_socket;
#else
    ip::tcp::socket & m_socket;
#endif
};

struct Writer : Task
{
    Writer(
        asio::io_service & ioService,

#ifdef SERVER_SSL
        ssl::stream< ip::tcp::socket > & socket    
#else
        ip::tcp::socket & socket
#endif

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

        while( std::cin.getline( m_request, m_maxLength ) )
        {
            std::size_t const size = strlen( m_request );

            asio::async_write(
                m_socket,
                asio::buffer( m_request, size ),
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

    enum { m_maxLength = 1024 + 2 };
    char m_request[ m_maxLength ];
};

struct Reader : Task
{
    Reader(
        asio::io_service & ioService,

#ifdef SERVER_SSL
        ssl::stream< ip::tcp::socket > & socket    
#else
        ip::tcp::socket & socket
#endif

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

#ifdef SERVER_SSL
        , m_ctx( ssl::context::sslv23 )
        , m_socket( m_ioService, m_ctx )
#else
        , m_socket( m_ioService )
#endif

        , m_reader( m_ioService, m_socket )
        , m_writer( m_ioService, m_socket )
    {
        ip::tcp::resolver::query query( host, port ); 

#ifdef SERVER_SSL
        m_ctx.load_verify_file( "ca.pem" );

        m_socket.set_verify_mode( ssl::verify_peer );
        m_socket.set_verify_callback(
            boost::bind( & Client::verifyCertificate, this, _1, _2 )
        );
#endif

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

#ifdef SERVER_SSL
    bool verifyCertificate(
        bool const preverified,
        ssl::verify_context & ctx 
    )
    {
        return preverified;
    }
#endif

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

#ifdef SERVER_SSL
                m_socket.lowest_layer(),
#else
                m_socket,
#endif

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
#ifdef SERVER_SSL
            auto const onHandShake = boost::bind(
                & Client::onHandShake,
                this,
                placeholders::error
            );

            m_socket.async_handshake(
                ssl::stream_base::client,
                onHandShake
            );

#else
            onHandShake( sys::error_code() );
#endif
        }
    }

    void onHandShake(
        sys::error_code const & errorCode
    )
    {
        if( errorCode )
        {
            std::cerr << "Handshake Error: " << errorCode.message() << std::endl;
        }
        else
        {
            m_reader.run();
        }
    }

    asio::io_service & m_ioService;

#ifdef SERVER_SSL
    ssl::context m_ctx;
    ssl::stream< ip::tcp::socket > m_socket;
#else
    ip::tcp::socket m_socket;
#endif

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

