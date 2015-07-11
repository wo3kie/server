#ifndef _CLIENT_HPP_
#define _CLIENT_HPP_

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
    );

    void run(){ runImpl(); }

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

Task::Task(
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

template< typename TReader, typename TWriter >
struct Client
{
    Client(
        asio::io_service & ioService,
        std::string const & host,
        std::string const & port,
        int argc = 0,
        char* argv[] = {}
    );

private:

#ifdef SERVER_SSL
    bool verifyCertificate(
        bool const preverified,
        ssl::verify_context & ctx 
    );
#endif

    void onResolved(
        sys::error_code const & errorCode,
        ip::tcp::resolver::iterator endpointIterator 
    );

    void onConnected(
        sys::error_code const & errorCode 
    );

    void onHandShake(
        sys::error_code const & errorCode
    );

    asio::io_service & m_ioService;

#ifdef SERVER_SSL
    ssl::context m_ctx;
    ssl::stream< ip::tcp::socket > m_socket;
#else
    ip::tcp::socket m_socket;
#endif

    TReader m_reader;
    TWriter m_writer;
};

template< typename TReader, typename TWriter >
Client< TReader, TWriter >::Client(
    asio::io_service & ioService,
    std::string const & host,
    std::string const & port,
    int argc,
    char* argv[]
)
    : m_ioService( ioService )

#ifdef SERVER_SSL
    , m_ctx( ssl::context::sslv23 )
    , m_socket( m_ioService, m_ctx )
#else
    , m_socket( m_ioService )
#endif

    , m_reader( m_ioService, m_socket, argc, argv )
    , m_writer( m_ioService, m_socket, argc, argv )
{
    ip::tcp::resolver::query query( host, port ); 

#ifdef SERVER_SSL
    m_ctx.load_verify_file( "./ca.pem" );

    m_socket.set_verify_mode( ssl::verify_peer );
    m_socket.set_verify_callback(
        boost::bind( & Client::verifyCertificate, this, _1, _2 )
    );
#endif

    ip::tcp::resolver resolver( m_ioService );

    auto const onResolved = boost::bind(
        & Client::onResolved,
        this,
        placeholders::error,
        placeholders::iterator
    );

    resolver.async_resolve(
        query,
        onResolved
    );

    boost::thread_group threadGroup; 

    auto const threadBody = boost::bind(
        & asio::io_service::run,
        & m_ioService
    );

    for( unsigned i = 0 ; i < 2 ; ++ i )
    {
        threadGroup.create_thread( threadBody );
    }

    threadGroup.join_all();
}

#ifdef SERVER_SSL
template< typename TReader, typename TWriter >
bool Client< TReader, TWriter >::verifyCertificate(
    bool const preverified,
    ssl::verify_context & ctx 
)
{
    return preverified;
}
#endif

template< typename TReader, typename TWriter >
void Client< TReader, TWriter >::onResolved(
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

template< typename TReader, typename TWriter >
void Client< TReader, TWriter >::onConnected(
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

template< typename TReader, typename TWriter >
void Client< TReader, TWriter >::onHandShake(
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
        boost::thread( boost::bind( & TWriter::run, m_writer ) ).join();
    }
}


#endif

