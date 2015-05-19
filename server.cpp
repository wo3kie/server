#include <set>
#include <sstream>
#include <string>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
namespace placeholders = asio::placeholders;
namespace sys = boost::system;
namespace algorithm = boost::algorithm;

class ConnectionManager;

class Connection
    : public boost::enable_shared_from_this< Connection >
{
public:

    typedef boost::shared_ptr< Connection > ConnectionPtr;

public:

    enum class Action
    {
        ReadCompleted,
        ReadError,
        ReadSome
    };

    Connection( 
        asio::io_service & ioService,
        ConnectionManager & connectionManager
    )
        : m_socket( ioService )
        , m_strand( ioService )
        , m_connectionManager( connectionManager )
    {
    }

    ~Connection()
    {
    }

    ip::tcp::socket & socket()
    {
        return m_socket;
    }

    void start(
        Action const action = Action::ReadSome
    )
    {
        auto const read = boost::bind(
            & Connection::read,
            shared_from_this(),
            placeholders::error,
            placeholders::bytes_transferred()
        );

        switch( action )
        {
            case Action::ReadCompleted:
            {
                process( 0 );
                
                break;
            }

            case Action::ReadError:
            {
                break;
            }

            case Action::ReadSome:
            {
                auto const read = boost::bind(
                    & Connection::read,
                    shared_from_this(),
                    placeholders::error,
                    placeholders::bytes_transferred()
                );

                m_socket.async_read_some(
                    asio::buffer( m_buffer, m_maxLength ),
                    m_strand.wrap( read )
                );

                break;
            }
        }
    }

    Action parse(
        char const * const begin,
        char const * const end
    )
    {
        return Action::ReadCompleted;
    }

    void processParseError(
        std::size_t const bytesTransferred
    )
    {
        std::cerr
            << "Parse error: "
            << std::string( m_buffer, bytesTransferred )
            << std::endl;
    }

    void process(
        std::size_t const bytesTransferred
    )
    {
        std::string text( m_buffer, bytesTransferred );

        if( text[0] == 'g' )
        {
            std::vector< std::string > tokens;
            algorithm::split( tokens, text, algorithm::is_any_of( " " ) );

            setId( tokens[1] );
            response( "Hello.", strlen( "Hello." ) );
        }
        else if( text[0] == 'b' )
        {
            broadcastAndStartAgain( text.c_str(), text.size() );
        }
        else if( text[0] == 'u' )
        {
            std::vector< std::string > tokens;
            algorithm::split( tokens, text, algorithm::is_any_of( " " ) );

            unicastAndStartAgain( tokens[1], tokens[2].c_str(), tokens[2].size() );
        }
        else
        {
            std::cout << "unknown" << std::endl;

            start();
        }
    }

    void broadcastAndStartAgain(
        char const * const message,
        std::size_t const size
    );

    void broadcastAndStop(
        char const * const message,
        std::size_t const size
    );

    void greetAndStartAgain(
        std::string const & id
    );

    void unicastAndStartAgain(
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    void unicastAndStop(
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    void response(
        char const * const message,
        std::size_t const size
    );

    void onStop();

    void setId(
        std::string const & id
    )
    {
        m_id = id;
    }

    std::string const & getId() const 
    {
        return m_id;
    }

private:

    void read(
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
            auto const result = parse( m_buffer, m_buffer + bytesTransferred );

            switch( result )
            {
                case Action::ReadCompleted:
                {
                    process( bytesTransferred );
                    
                    break;
                }

                case Action::ReadError:
                {
                    processParseError( bytesTransferred );

                    break;
                }

                case Action::ReadSome:
                {
                    auto const read = boost::bind(
                        & Connection::read,
                        shared_from_this(),
                        placeholders::error,
                        placeholders::bytes_transferred()
                    );

                    m_socket.async_read_some(
                        asio::buffer( m_buffer, m_maxLength ),
                        m_strand.wrap( read )
                    );

                    break;
                }
            }
        }
    }

    void startAgain(
        sys::error_code const & errorCode
    )
    {
        if( ! errorCode )
        {
            start();
        }
    }

    void stop(
        sys::error_code const & errorCode
    )
    {
    }

private:

    ip::tcp::socket m_socket;
    asio::io_service::strand m_strand;
    ConnectionManager & m_connectionManager;
    std::string m_id;

    enum { m_maxLength = 1024 };
    char m_buffer[ m_maxLength ];
};

typedef Connection::ConnectionPtr ConnectionPtr;
    
class ConnectionManager
    : public boost::noncopyable
{
    typedef std::set< ConnectionPtr > ConnectionsPtr;

public:

    template< typename Function, class... Args >
    void forEach( Function&& func, Args && ...args )
    {
        boost::lock_guard< boost::mutex > lock( m_mutex );

        for( auto & connection : m_connections )
        {
            func( connection, std::forward< Args >( args )... );        
        }
    }

    template< typename Predicate, typename Function, class... Args >
    void forEachIf( Predicate && predicate, Function && func, Args && ...args )
    {
        boost::lock_guard< boost::mutex > lock( m_mutex );

        for( auto & connection : m_connections )
        {
            if( predicate( connection ) )
            {
                func( connection, std::forward< Args >( args )... );        
            }
        }
    }

    void add(
        ConnectionPtr & connection
    )
    {
        boost::lock_guard< boost::mutex > lock( m_mutex );

        m_connections.insert( connection ); 
    }

    void remove(
        ConnectionPtr const & connection
    )
    {
        boost::lock_guard< boost::mutex > lock( m_mutex );

        auto const pos = m_connections.find( connection );

        if( pos != m_connections.end() ){
            m_connections.erase( pos );
        }
    }

private:

    boost::mutex m_mutex;
    ConnectionsPtr m_connections;
};

void Connection::onStop()
{
    char const * const message = "Goodbye.";

    response( message, strlen( message ) );
}

void Connection::response(
    char const * const message,
    std::size_t const size
)
{
    auto const startAgain = boost::bind(
        & Connection::startAgain,
        shared_from_this(),
        placeholders::error
    );

    asio::async_write(
        m_socket,
        asio::buffer( message, size ),
        startAgain 
    );
}

void Connection::broadcastAndStop(
    char const * const message,
    std::size_t const size
)
{
    auto const skipSender = [ this ]( ConnectionPtr const & connectionPtr )
    {
        return connectionPtr->socket().native_handle() != this->socket().native_handle();
    };

    auto sendMessage = [ this, & size, & message ]( ConnectionPtr const & connectionPtr )
    {
        auto const stop = boost::bind(
            & Connection::stop,
            shared_from_this(),
            placeholders::error
        );

        asio::async_write(
            connectionPtr->socket(),
            asio::buffer( message, size ),
            stop
        );
    };

    m_connectionManager.forEachIf( skipSender, sendMessage );
}

void Connection::broadcastAndStartAgain(
    char const * const message,
    std::size_t const size
)
{
    auto const skipSender = [ this ]( ConnectionPtr const & connectionPtr )
    {
        return connectionPtr->socket().native_handle() != this->socket().native_handle();
    };

    auto sendMessage = [ this, & size, & message ]( ConnectionPtr const & connectionPtr )
    {
        auto const startAgain = boost::bind(
            & Connection::startAgain,
            shared_from_this(),
            placeholders::error
        );

        asio::async_write(
            connectionPtr->socket(),
            asio::buffer( message, size ),
            startAgain
        );
    };

    m_connectionManager.forEachIf( skipSender, sendMessage );
}

void Connection::unicastAndStop(
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    auto const matchReceiver = [ this, & receiverId ]( ConnectionPtr const & connectionPtr )
    {
        return connectionPtr->getId() == receiverId;
    };

    auto sendMessage = [ this, & size, & message ]( ConnectionPtr const & connectionPtr )
    {
        auto const stop = boost::bind(
            & Connection::stop,
            shared_from_this(),
            placeholders::error
        );

        asio::async_write(
            connectionPtr->socket(),
            asio::buffer( message, size ),
            stop
        );
    };

    m_connectionManager.forEachIf( matchReceiver, sendMessage );
}

void Connection::unicastAndStartAgain(
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    auto const matchReceiver = [ this, & receiverId ]( ConnectionPtr const & connectionPtr )
    {
        return connectionPtr->getId() == receiverId;
    };

    auto sendMessage = [ this, & size, & message ]( ConnectionPtr const & connectionPtr )
    {
        auto const startAgain = boost::bind(
            & Connection::startAgain,
            shared_from_this(),
            placeholders::error
        );

        asio::async_write(
            connectionPtr->socket(),
            asio::buffer( message, size ),
            startAgain
        );
    };

    m_connectionManager.forEachIf( matchReceiver, sendMessage );
}

void Connection::greetAndStartAgain(
    std::string const & name
)
{
    setId( name );

    auto const startAgain = boost::bind(
        & Connection::startAgain,
        shared_from_this(),
        placeholders::error
    );

    start();
}

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
        m_newConnection.reset( new Connection( m_ioService, m_connectionManager ) );

        m_connectionManager.add( m_newConnection );

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
        m_connectionManager.forEach(
            boost::bind( & Connection::onStop, _1 )
        );

        m_ioService.stop();
    }

private:

    asio::io_service m_ioService;
    asio::ip::tcp::acceptor m_acceptor;
    asio::signal_set m_signals;

    ConnectionPtr m_newConnection;
    ConnectionManager m_connectionManager;
};

int main( int argc, char* argv[] )
{
    Server( argv[1] ).run();
}

