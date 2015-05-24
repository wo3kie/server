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

template< typename TConnection >
class Task
{
public:
    Task( TConnection * connection )
        : m_connection( connection )
    {
    }

    static typename TConnection::Action start()
    {
        return TConnection::Action::Read;
    }

    typename TConnection::Action parse(
        char const * const buffer,
        std::size_t const bytesTransferred
    )
    {
        std::istringstream iss( std::string( buffer, bytesTransferred ) );

        char command, space, cr, lf;
        iss >> command;

        if( command == 'g' )
        {
            std::string id;
            iss >> id;
            m_connection->setId( id );

            const std::string message = "Hello " + id;

            m_connection->response( message.c_str(), message.size() );
            m_connection->read();
        }
        else if( command == 'b' )
        {
            std::string message;
            iss >> message;

            m_connection->broadcast( message.c_str(), message.size() );
            m_connection->read();
        }
        else if( command == 'u' )
        {
            std::string id;
            std::string message;
            iss >> id >> message;

            m_connection->unicast( id, message.c_str(), message.size() );
            m_connection->read();
        }
        else if( command == 'd' )
        {
            m_connection->disconnect();
        }
        else if( command == 'r' )
        {
            std::string message;
            iss >> message;

            m_connection->response( message.c_str(), message.size() );
            m_connection->read();
        }
        else
        {
            char const * const message = "unknown";
            m_connection->response( message, strlen( message ) );
            m_connection->read();
        }

        return TConnection::Action::Process;
    }

    void parseError()
    {
        std::cerr
            << "Parse error"
            << std::endl;
    }

    void process()
    {
    }

private:
    TConnection * m_connection;
};

class ConnectionManager;

class Connection
    : public boost::enable_shared_from_this< Connection >
{
public:

    typedef boost::shared_ptr< Connection > ConnectionPtr;

public:

    enum class Action
    {
        Read,
        ReadError,
        Process
    };

    Connection( 
        asio::io_service & ioService,
        ConnectionManager & connectionManager
    );

    ip::tcp::socket & socket();

    void disconnect();

    void stop();

    void start(
        Action const action = Task< Connection >::start()
    );

public: // api

    void setId(
        std::string const & id
    );

    std::string const & getId() const ;

    void read();

    void parseError();

    void process();

    void response(
        char const * const message,
        std::size_t const size
    );

    void unicast(
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    void broadcast(
        char const * const message,
        std::size_t const size
    );

private:

    void parse(
        sys::error_code const & errorCode,
        std::size_t const bytesTransferred
    );

    void startAgain(
        sys::error_code const & errorCode
    );

    void doNothing(
        sys::error_code const & errorCode
    )
    {
        if( errorCode )
        {
            std::cerr << "Do Nothing Error: " << errorCode.message() << std::endl;

            disconnect();
        }
    }

private:

    ip::tcp::socket m_socket;
    asio::io_service::strand m_strand;
    ConnectionManager & m_connectionManager;

    Task< Connection > m_task;

    std::string m_id;

    enum { m_maxSize = 1024 };
    char m_buffer[ m_maxSize ];
};


Connection::Connection( 
    asio::io_service & ioService,
    ConnectionManager & connectionManager
)
    : m_socket( ioService )
    , m_strand( ioService )
    , m_connectionManager( connectionManager )
    , m_task( this )
{
}

ip::tcp::socket & Connection::socket()
{
    return m_socket;
}

void Connection::setId(
    std::string const & id
)
{
    m_id = id;
}

std::string const & Connection::getId() const 
{
    return m_id;
}

void Connection::start(
    Connection::Action const action
)
{
    switch( action )
    {
        case Action::Process:
        {
            process();
            
            break;
        }

        case Action::ReadError:
        {
            parseError();

            break;
        }

        case Action::Read:
        {
            auto const parse = boost::bind(
                & Connection::parse,
                shared_from_this(),
                placeholders::error,
                placeholders::bytes_transferred()
            );

            m_socket.async_read_some(
                asio::buffer( m_buffer, m_maxSize ),
                m_strand.wrap( parse )
            );

            break;
        }
    }
}

void Connection::read()
{
    start( Action::Read );
}

void Connection::parseError()
{
    m_task.parseError();
}

void Connection::process()
{
    m_task.process();
}

void Connection::parse(
    sys::error_code const & errorCode,
    std::size_t const bytesTransferred
)
{
    if( errorCode )
    {
        std::cerr << "Parse Error: " << errorCode.message() << std::endl;

        disconnect();
    }
    else
    {
        start( m_task.parse( m_buffer, bytesTransferred ) );
    }
}

void Connection::startAgain(
    sys::error_code const & errorCode
)
{
    if( errorCode )
    {
        std::cerr << "Start Again Error: " << errorCode.message() << std::endl;

        disconnect();
    }
    else
    {
        start();
    }
}

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

void Connection::disconnect()
{
    m_connectionManager.remove( shared_from_this() );
}

void Connection::response(
    char const * const message,
    std::size_t const size
)
{
    auto const continuation = boost::bind(
        & Connection::doNothing,
        shared_from_this(),
        placeholders::error
    );

    asio::async_write(
        m_socket,
        asio::buffer( message, size ),
        continuation
    );
}

void Connection::unicast(
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
        auto const continuation = boost::bind(
            & Connection::doNothing,
            shared_from_this(),
            placeholders::error
        );

        asio::async_write(
            connectionPtr->socket(),
            asio::buffer( message, size ),
            continuation
        );
    };

    m_connectionManager.forEachIf( matchReceiver, sendMessage );
}

void Connection::broadcast(
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
        auto const continuation = boost::bind(
            & Connection::doNothing,
            shared_from_this(),
            placeholders::error
        );

        asio::async_write(
            connectionPtr->socket(),
            asio::buffer( message, size ),
            continuation
        );
    };

    m_connectionManager.forEachIf( skipSender, sendMessage );
}

void Connection::stop()
{
    char const * const message = "Goodbye.";
    response( message, strlen( message ) );
}

class Server
{
public:

    Server(
        std::string const & port
    )
        : m_strand( m_ioService )
        , m_acceptor( m_ioService )
        , m_signals( m_ioService )
    {
        m_signals.add( SIGINT );
        m_signals.add( SIGTERM );
        m_signals.add( SIGQUIT );

        m_signals.async_wait(
            boost::bind( & Server::stop, this )
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
            m_connectionManager.add( m_newConnection );

            m_newConnection->start();
        }

        startAccept();
    }

    void stop()
    {
        m_ioService.stop();
    }

private:

    asio::io_service m_ioService;
    asio::io_service::strand m_strand;
    asio::ip::tcp::acceptor m_acceptor;
    asio::signal_set m_signals;

    ConnectionPtr m_newConnection;
    ConnectionManager m_connectionManager;
};

int main( int argc, char* argv[] )
{
    Server( argv[1] ).run();
}

