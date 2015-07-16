#ifndef _CORE_CLIENT_HPP_
#define _CORE_CLIENT_HPP_

#include <iostream>
#include <string>

#include <boost/asio.hpp>

#ifdef SERVER_SSL
    #include <boost/asio/ssl.hpp>
#endif

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

inline
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

template<
    typename TReader,
    typename TWriter
>
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

    #ifdef SERVER_SSL
        bool verifyCertificate(
            bool const preverified,
            ssl::verify_context & ctx 
        );
    #endif

private:

    asio::io_service & m_ioService;

    TReader m_reader;
    TWriter m_writer;

    #ifdef SERVER_SSL
        ssl::context m_ctx;
        ssl::stream< ip::tcp::socket > m_socket;
    #else
        ip::tcp::socket m_socket;
    #endif
};

template<
    typename TReader,
    typename TWriter
>
Client< TReader, TWriter >::Client(
    asio::io_service & ioService,
    std::string const & host,
    std::string const & port,
    int argc,
    char* argv[]
)
    : m_ioService( ioService )
    , m_reader( m_ioService, m_socket, argc, argv )
    , m_writer( m_ioService, m_socket, argc, argv )

    #ifdef SERVER_SSL
        , m_ctx( ssl::context::sslv23 )
        , m_socket( m_ioService, m_ctx )
    #else
        , m_socket( m_ioService )
    #endif
{
    ip::tcp::resolver::query query( host, port ); 

    #ifdef SERVER_SSL
        
        m_ctx.load_verify_file( "../pem/ca.pem" );
    
        m_socket.set_verify_mode( ssl::verify_peer );
        
        auto const verifyCallback = [ this ](
            bool const preverified,
            ssl::verify_context & ctx
        ) -> bool
        {
            return this->verifyCertificate( preverified, ctx );
        };

        m_socket.set_verify_callback( verifyCallback );

    #endif

    ip::tcp::resolver resolver( m_ioService );

    auto const onResolved = [ this ](
        sys::error_code const & errorCode,
        ip::tcp::resolver::iterator endpointIterator 
    )
    {
        this->onResolved( errorCode, endpointIterator );
    };

    resolver.async_resolve(
        query,
        onResolved
    );

    boost::thread_group threadGroup; 

    auto const threadBody = [ this ]()
    {
        this->m_ioService.run();
    };

    for( unsigned i = 0 ; i < 2 ; ++ i )
    {
        threadGroup.create_thread( threadBody );
    }

    threadGroup.join_all();
}

#ifdef SERVER_SSL

    template<
        typename TReader,
        typename TWriter
    >
    bool Client< TReader, TWriter >::verifyCertificate(
        bool const preverified,
        ssl::verify_context & ctx 
    )
    {
        return preverified;
    }

#endif

template<
    typename TReader,
    typename TWriter
>
void Client< TReader, TWriter >::onResolved(
    sys::error_code const & errorCode,
    ip::tcp::resolver::iterator endpointIterator 
)
{
    if( errorCode )
    {
        std::cerr << "Client::onResolved Error: " << errorCode.message() << std::endl;
    }
    else
    {
        auto const onConnected = [ this ](
            sys::error_code const & errorCode,
            ip::tcp::resolver::iterator const endpointIterator 
        )
        {
            this->onConnected( errorCode );
        };

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

template<
    typename TReader,
    typename TWriter
>
void Client< TReader, TWriter >::onConnected(
    sys::error_code const & errorCode 
)
{
    if( errorCode )
    {
        std::cerr << "Client::onConnected Error: " << errorCode.message() << std::endl;
    }
    else
    {
        #ifdef SERVER_SSL

            auto const onHandShake = [ this ](
                sys::error_code const & errorCode 
            )
            {
                this->onHandShake( errorCode );
            };
    
            m_socket.async_handshake(
                ssl::stream_base::client,
                onHandShake
            );

        #else
            onHandShake( sys::error_code() );
        #endif
    }
}

template<
    typename TReader,
    typename TWriter
>
void Client< TReader, TWriter >::onHandShake(
    sys::error_code const & errorCode
)
{
    if( errorCode )
    {
        std::cerr << "Client::onHandshake Error: " << errorCode.message() << std::endl;
    }
    else
    {
        auto const threadBody = [ this ]()
        {
            this->m_writer.run();
        };

        m_reader.run();
        boost::thread( threadBody ).join();
    }
}

#endif

