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
        asio::streambuf & buffer
    )
    {
        return TConnection::Action::ReadCompleted;
    }

    typename TConnection::Action parseSome(
        asio::streambuf & buffer
    )
    {
        return TConnection::Action::ReadCompleted;
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
            m_connection->responseAndRead( line, strlen( line ) );
        }
        else if( command == 'b' )
        {
            std::string message;
            iss >> std::noskipws >> space >>  message >> cr >> lf;

            m_connection->broadcastAndRead( message.c_str(), message.size() );
        }
        else if( command == 'u' )
        {
            std::string id;
            std::string message;
            iss >> std::noskipws >> space >> id >> space >> message >> cr >> lf;

            m_connection->unicastAndRead( id, message.c_str(), message.size() );
        }
        else
        {
            char const * const line = "unknown";

            m_connection->responseAndRead( line, strlen( line ) );
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
        ReadCompleted,
        ReadError,
        ReadLine,
        ReadSome
    };

    Connection( 
        asio::io_service & ioService,
        ConnectionManager & connectionManager
    );

    ip::tcp::socket & socket();

    void onStop();

    void start(
        Action const action = Task< Connection >::start()
    );

public: // api

    void setId(
        std::string const & id
    );

    std::string const & getId() const ;

    Action parseLine();

    Action parseSome();

    void parseError(
        sys::error_code const & errorCode
    );

    void process(
        sys::error_code const & errorCode
    );

    void broadcastAndRead(
        char const * const message,
        std::size_t const size
    );

    void broadcastAndProcess(
        char const * const message,
        std::size_t const size
    );

    void broadcastAndStop(
        char const * const message,
        std::size_t const size
    );

    void unicastAndRead(
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    void unicastAndStop(
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    void unicastAndProcess(
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    void responseAndRead(
        char const * const message,
        std::size_t const size
    );

    void responseAndProcess(
        char const * const message,
        std::size_t const size
    );

    void responseAndStop(
        char const * const message,
        std::size_t const size
    );

private:

    template< typename MemberPtr >
    void broadcastImpl(
        MemberPtr && memberPtr,
        char const * const message,
        std::size_t const size
    );

    template< typename MemberPtr >
    void unicastImpl(
        MemberPtr && memberPtr,
        std::string const & receiverId,
        char const * const message,
        std::size_t const size
    );

    template< typename MemberPtr >
    void responseImpl(
        MemberPtr && memberPtr,
        char const * const message,
        std::size_t const size
    );

    void readLine(
        sys::error_code const & errorCode,
        std::size_t const bytesTransferred
    );

    void readSome(
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
        case Action::ReadCompleted:
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
            auto const readLine = boost::bind(
                & Connection::readLine,
                shared_from_this(),
                placeholders::error,
                placeholders::bytes_transferred()
            );

            asio::async_read_until(
                m_socket,
                m_buffer,
                "\r\n",
                m_strand.wrap( readLine )
            );

            break;
        }

        case Action::ReadSome:
        {
            auto const readSome = boost::bind(
                & Connection::readSome,
                shared_from_this(),
                placeholders::error,
                placeholders::bytes_transferred()
            );

            asio::async_read(
                m_socket,
                m_buffer,
                asio::transfer_at_least( 1 ),
                m_strand.wrap( readSome )
            );

            break;
        }
    }
}

Connection::Action Connection::parseLine()
{
    return m_task.parseLine( m_buffer );
}

Connection::Action Connection::parseSome()
{
    return m_task.parseLine( m_buffer );
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

void Connection::readLine(
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
        start( parseLine() );
    }
}

void Connection::readSome(
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
        start( parseSome() );
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

void Connection::stop(
    sys::error_code const & errorCode
)
{
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

void Connection::onStop()
{
    char const * const message = "Goodbye.";

    responseAndStop( message, strlen( message ) );
}

template< typename MemberPtr >
void Connection::responseImpl(
    MemberPtr && memberPtr,
    char const * const message,
    std::size_t const size
)
{
    auto const continuation = boost::bind(
        std::forward< MemberPtr >( memberPtr ),
        shared_from_this(),
        placeholders::error
    );

    asio::async_write(
        m_socket,
        asio::buffer( message, size ),
        continuation
    );
}

void Connection::responseAndRead(
    char const * const message,
    std::size_t const size
)
{
    responseImpl( & Connection::startAgain, message, size );
}

void Connection::responseAndProcess(
    char const * const message,
    std::size_t const size
)
{
    responseImpl( & Connection::process, message, size );
}

void Connection::responseAndStop(
    char const * const message,
    std::size_t const size
)
{
    responseImpl( & Connection::stop, message, size );
}

template< typename MemberPtr >
void Connection::broadcastImpl(
    MemberPtr && memberPtr,
    char const * const message,
    std::size_t const size
)
{
    auto const skipSender = [ this ]( ConnectionPtr const & connectionPtr )
    {
        return connectionPtr->socket().native_handle() != this->socket().native_handle();
    };

    auto sendMessage = [ this, & size, & message, & memberPtr ]( ConnectionPtr const & connectionPtr )
    {
        auto const continuation = boost::bind(
            std::forward< MemberPtr >( memberPtr ),
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

void Connection::broadcastAndStop(
    char const * const message,
    std::size_t const size
)
{
    broadcastImpl( & Connection::stop, message, size );
}

void Connection::broadcastAndRead(
    char const * const message,
    std::size_t const size
)
{
    broadcastImpl( & Connection::startAgain, message, size );
}

void Connection::broadcastAndProcess(
    char const * const message,
    std::size_t const size
)
{
    broadcastImpl( & Connection::process, message, size );
}

template< typename MemberPtr >
void Connection::unicastImpl(
    MemberPtr && memberPtr,
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    auto const matchReceiver = [ this, & receiverId ]( ConnectionPtr const & connectionPtr )
    {
        return connectionPtr->getId() == receiverId;
    };

    auto sendMessage = [ this, & size, & message, & memberPtr ]( ConnectionPtr const & connectionPtr )
    {
        auto const stop = boost::bind(
            std::forward< MemberPtr >( memberPtr ),
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

void Connection::unicastAndStop(
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    unicastImpl( & Connection::stop, receiverId, message, size );
}

void Connection::unicastAndProcess(
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    unicastImpl( & Connection::process, receiverId, message, size );
}

void Connection::unicastAndRead(
    std::string const & receiverId,
    char const * const message,
    std::size_t const size
)
{
    unicastImpl( & Connection::startAgain, receiverId, message, size );
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

