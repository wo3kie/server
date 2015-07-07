#ifndef _CLIENT_KEY_
#define _CLIENT_KEY_

#include <fstream>

#include "./client.hpp"

struct WriterKey : Task
{
    WriterKey(
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
        auto const onRequestWritten = boost::bind(
            & WriterKey::onRequestWritten,
            this,
            placeholders::error,
            placeholders::bytes_transferred
        );

        m_request[ 0 ] = 'k';

        asio::async_write(
            m_socket,
            asio::buffer( m_request, 1 ),
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
};

struct ReaderKey : Task
{
    ReaderKey(
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
            & ReaderKey::onResponseRead,
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

