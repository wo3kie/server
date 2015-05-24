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
        return TConnection::Action::ReadLine;
    }

    typename TConnection::Action parseLine(
        asio::streambuf & buffer,
        std::size_t const bytesTransferred
    )
    {
        return TConnection::Action::Process;
    }

    typename TConnection::Action parseSome(
        asio::streambuf & buffer,
        std::size_t const bytesTransferred
    )
    {
        return TConnection::Action::Process;
    }

    void parseError(
        sys::error_code const & errorCode
    )
    {
        std::cerr
            << "Parse error"
            << std::endl;
    }

    void process(
        asio::streambuf & buffer,
        sys::error_code const & errorCode
    )
    {
        std::istream iss( & buffer );

        char command, space, cr, lf;
        iss >> command;

        if( command == 'g' )
        {
            std::string id;
            iss >> std::noskipws >> space >> id >> cr >> lf;
            m_connection->setId( id );

            char const * const line = "Hello.";
            m_connection->response( line, strlen( line ) );
            m_connection->start( TConnection::Action::ReadLine );
        }
        else if( command == 'b' )
        {
            std::string message;
            iss >> std::noskipws >> space >>  message >> cr >> lf;

            m_connection->broadcast( message.c_str(), message.size() );
            m_connection->start( TConnection::Action::ReadLine );
        }
        else if( command == 'u' )
        {
            std::string id;
            std::string message;
            iss >> std::noskipws >> space >> id >> space >> message >> cr >> lf;

            m_connection->unicast( id, message.c_str(), message.size() );
            m_connection->start( TConnection::Action::ReadLine );
        }
        else if( command == 'd' )
        {
            iss >> std::noskipws >> cr >> lf;

            m_connection->disconnect();
        }
        else
        {
            char const * const line = "unknown";
            m_connection->response( line, strlen( line ) );
            m_connection->start( TConnection::Action::ReadLine );
        }
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
        Process,
        ReadLine,
        ReadSome,
        ReadError
    };

    Connection( 
        asio::io_service & ioService,
        ConnectionManager & connectionManager
    );

    ip::tcp::socket & socket();

    void disconnect();

    void start(
        Action const action = Task< Connection >::start()
    );

public: // api

    void setId(
        std::string const & id
    );

    std::string const & getId() const ;

    void parseError(
        sys::error_code const & errorCode
    );

    void process(
        sys::error_code const & errorCode
    );

    void broadcast(
        char const * const message,
        std::size_t const size
    );

    void unicast(
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    void response(
        char const * const message,
        std::size_t const size
    );

    void doNothing(
        sys::error_code const & errorCode
    )
    {
    }

private:

    void parseLine(
        sys::error_code const & errorCode,
        std::size_t const bytesTransferred
    );

    void parseSome(
        sys::error_code const & errorCode,
        std::size_t const bytesTransferred
    );

    void startAgain(
        sys::error_code const & errorCode
    );

    void stop(
        sys::error_code const & errorCode
    );

private:

    ip::tcp::socket m_socket;
    asio::io_service::strand m_strand;
    ConnectionManager & m_connectionManager;

    Task< Connection > m_task;

    std::string m_id;

    asio::streambuf m_buffer;
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
            process( sys::error_code() );
            
            break;
        }

        case Action::ReadError:
        {
            break;
        }

        case Action::ReadLine:
        {
            auto const parseLine = boost::bind(
                & Connection::parseLine,
                shared_from_this(),
                placeholders::error,
                placeholders::bytes_transferred()
            );

            asio::async_read_until(
                m_socket,
                m_buffer,
                "\r\n",
                m_strand.wrap( parseLine )
            );

            break;
        }

        case Action::ReadSome:
        {
            auto const parseSome = boost::bind(
                & Connection::parseSome,
                shared_from_this(),
                placeholders::error,
                placeholders::bytes_transferred()
            );

            asio::async_read(
                m_socket,
                m_buffer,
                asio::transfer_at_least( 1 ),
                m_strand.wrap( parseSome )
            );

            break;
        }
    }
}

void Connection::parseError(
    sys::error_code const & errorCode
)
{
    m_task.parseError( errorCode );
}

void Connection::process(
    sys::error_code const & errorCode
)
{
    m_task.process( m_buffer, errorCode );
}

void Connection::parseLine(
    sys::error_code const & errorCode,
    std::size_t const bytesTransferred
)
{
    if( errorCode )
    {
        std::cerr << "Parse Line Error: " << errorCode.message() << std::endl;
    }
    else
    {
        start( m_task.parseLine( m_buffer, bytesTransferred ) );
    }
}

void Connection::parseSome(
    sys::error_code const & errorCode,
    std::size_t const bytesTransferred
)
{
    if( errorCode )
    {
        std::cerr << "Parse Some Error: " << errorCode.message() << std::endl;
    }
    else
    {
        start( m_task.parseSome( m_buffer, bytesTransferred ) );
    }
}

void Connection::startAgain(
    sys::error_code const & errorCode
)
{
    if( errorCode )
    {
        std::cerr << "Start Again Error: " << errorCode.message() << std::endl;
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
    char const * const message = "Goodbye.";

    response( message, strlen( message ) );
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
        auto const stop = boost::bind(
            & Connection::doNothing,
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

void Connection::stop(
    sys::error_code const & errorCode
)
{
    m_connectionManager.remove( shared_from_this() );
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
            boost::bind( & Server::disconnect, this )
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

    void disconnect()
    {
        m_connectionManager.forEach(
            boost::bind( & Connection::disconnect, _1 )
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

