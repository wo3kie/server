#ifndef _CLIENT_SIGN_
#define _CLIENT_SIGN_

#include <fstream>

#include "../core/client.hpp"

struct WriterSign : Task
{
    WriterSign(
        asio::io_service & ioService,
        ssl::stream< ip::tcp::socket > & socket,
        int argc,
        char* argv[]
    )
        : Task( ioService, socket )
        , m_fileName( argv[ 3 ] )
    {
    }

protected:

    void runImpl() override
    {
        auto const onRequestWritten = boost::bind(
            & WriterSign::onRequestWritten,
            this,
            placeholders::error,
            placeholders::bytes_transferred
        );

        SHA_CTX ctx;
        SHA1_Init( & ctx );

        char buffer[ 4 * 1024 ];
        std::ifstream fileIn( m_fileName.c_str(), std::ios_base::binary );

        while( fileIn.read( & buffer[ 0 ], sizeof( buffer ) ) )
        {
            SHA1_Update( & ctx, & buffer[ 0 ], sizeof( buffer ) );
        }
            
        SHA1_Update( & ctx, & buffer[ 0 ], fileIn.gcount() );

        SHA1_Final( (unsigned char*)( & m_request[ 2 ] ), & ctx );

        m_request[ 0 ] = 's';
        m_request[ 1 ] = ' ';

        asio::async_write(
            m_socket,
            asio::buffer( m_request, SHA_DIGEST_LENGTH + 2 ),
            onRequestWritten
        );
    }

private:

    void onRequestWritten(
        sys::error_code const & errorCode,
        int bytesTransferred
    )
    {
        if( errorCode )
        {
            std::cerr << "Request Write Error: " << errorCode.value() << " " << errorCode.message() << std::endl;
        }
    }

private:

    enum { m_maxLength = 1024 };
    char m_request[ m_maxLength ];

    std::string m_fileName;
};

struct ReaderSign : Task
{
    ReaderSign(
        asio::io_service & ioService,
        ssl::stream< ip::tcp::socket > & socket,
        int argc,
        char* argv[]
    )
        : Task( ioService, socket )
    {
    }

protected:

    void runImpl() override
    {
        auto const onResponseRead = boost::bind(
            & ReaderSign::onResponseRead,
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
            std::cout.write( m_response, bytesTransferred );
            std::flush( std::cout );
        }
    }

private:

    enum { m_maxLength = 1024 };
    char m_response[ m_maxLength ];
};

#endif

