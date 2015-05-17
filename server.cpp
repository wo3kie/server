#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;

class Connection
    : public boost::enable_shared_from_this< Connection >
{
public:
    
    Connection( 
        asio::io_service & ioService
    )
        : m_socket( ioService )
        , m_strand( ioService )
    {
    }

    ~Connection()
    {
    }

    ip::tcp::socket & socket()
    {
        return m_socket;
    }

    void start()
    {
        auto const onDataRead = boost::bind(
            & Connection::onDataRead,
            shared_from_this(),
            placeholders::error,
            placeholders::bytes_transferred()
        );

        m_socket.async_read_some(
            asio::buffer( m_request, m_maxLength ),
            m_strand.wrap( onDataRead )
        );
    }

    void onDataRead(
        sys::error_code const & errorCode,
        std::size_t const bytesTransferred
    )
    {
        if( errorCode )
        {
            std::cerr << "Response Read Error: " << errorCode.message() << std::endl;
        }
        else
        {
            std::cout.write( m_request, bytesTransferred );
            std::cout << std::endl;

            auto const onDataWritten = boost::bind(
                & Connection::onDataWritten,
                shared_from_this(),
                placeholders::error
            );

            asio::async_write(
                m_socket,
                asio::buffer( m_request, strlen( m_request ) ),
                onDataWritten
            );
        }
    }

    void onDataWritten(
        sys::error_code const & errorCode
    )
    {
        if( ! errorCode )
        {
            start();
        }
    }

private:

    ip::tcp::socket m_socket;
    asio::io_service::strand m_strand;

    enum { m_maxLength = 1024 };
    char m_request[ m_maxLength ];
};

class Server
{
public:
    
    Server(
        std::string const & port
    )
        : m_acceptor( m_ioService )
        , m_signals( m_ioService )
    {
        m_signals.add( SIGINT );
        m_signals.add( SIGTERM );
        m_signals.add( SIGQUIT );

        m_signals.async_wait(
            boost::bind( & Server::onStop, this )
        );

        ip::tcp::resolver resolver( m_ioService );
        ip::tcp::resolver::query query( "127.0.0.1", port );
        ip::tcp::endpoint endpoint = * resolver.resolve( query );

        m_acceptor.open( endpoint.protocol() );
        m_acceptor.set_option( ip::tcp::acceptor::reuse_address( true ) );
        m_acceptor.bind( endpoint );
        m_acceptor.listen();

        startAccept();
    }

    void run()
    {
        boost::thread_group threadGroup; 

        auto const threadBody = boost::bind(
            & asio::io_service::run,
            & m_ioService
        );

        for( unsigned i = 0 ; i < 4 ; ++ i )
        {
            threadGroup.create_thread( threadBody );
        }

        threadGroup.join_all();
    }

private:

    void startAccept()
    {
        m_newConnection.reset( new Connection( m_ioService ) );

        auto const onAccepted = boost::bind(
            & Server::onAccepted,
            this,
            placeholders::error
        );

        m_acceptor.async_accept(
            m_newConnection->socket(),
            onAccepted
        );
    }

    void onAccepted(
        sys::error_code const & errorCode
    )
    {
        if( ! errorCode )
        {
            m_newConnection->start();
        }

        startAccept();
    }

    void onStop()
    {
        m_ioService.stop();
    }

private:

    asio::io_service m_ioService;
    asio::ip::tcp::acceptor m_acceptor;
    asio::signal_set m_signals;

    boost::shared_ptr< Connection > m_newConnection;
};

int main( int argc, char* argv[] )
{
    Server( argv[1] ).run();
}

