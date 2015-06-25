#include "./client.hpp"

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

void Task::run()
{
    auto const threadBody = boost::bind(
        & asio::io_service::run,
        & m_ioService
    );

    boost::thread thread( threadBody );

    runImpl();

    thread.join();
}

template< typename TReader, typename TWriter >
Client< TReader, TWriter >::Client(
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

    boost::thread( boost::bind( & TWriter::run, m_writer ) ).join();
}

#ifdef SERVER_SSL
template< typename TReader, typename TWriter >
void Client< TReader, TWriter >::verifyCertificate(
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
    }
}

