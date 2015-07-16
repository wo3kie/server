#ifndef _CLIENT_CONSOLE_
#define _CLIENT_CONSOLE_

#include "core/client.hpp"

struct Writer : Task
{
    Writer(
        asio::io_service & ioService,

        #ifdef SERVER_SSL
            ssl::stream< ip::tcp::socket > & socket,
        #else
            ip::tcp::socket & socket,
        #endif

        int argc,
        char* argv[]
    )
        : Task( ioService, socket )
    {
    }

protected:

    void runImpl() override
    {
        auto const onRequestWritten = [ this ](
            sys::error_code const & errorCode,
            size_t const bytesTransferred
        )
        {
            this->onRequestWritten( errorCode );
        };

        while( std::cin.getline( m_request, m_maxLength ) )
        {
            std::size_t const size = strlen( m_request );

            asio::async_write(
                m_socket,
                asio::buffer( m_request, size ),
                onRequestWritten
            );
        }
    }

private:

    void onRequestWritten(
        sys::error_code const & errorCode
    )
    {
        if( errorCode )
        {
            std::cerr 
                << "ClientConsole::onRequestWritten Error: "
                << errorCode.message() << std::endl;
        }
    }

private:

    enum { m_maxLength = 1024 + 2 };
    char m_request[ m_maxLength ];
};

struct Reader : Task
{
    Reader(
        asio::io_service & ioService,

        #ifdef SERVER_SSL
            ssl::stream< ip::tcp::socket > & socket,
        #else
            ip::tcp::socket & socket,
        #endif

        int argc,
        char* argv[]
    )
        : Task( ioService, socket )
    {
    }

protected:

    void runImpl() override
    {
        auto const onResponseRead = [ this ](
            sys::error_code const & errorCode,
            size_t const bytesTransferred
        )
        {
            this->onResponseRead( errorCode, bytesTransferred );
        };

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
            std::cerr
                << "ClientConsole::onResponseRead Error: "
                << errorCode.message() << std::endl;
        }
        else
        {
            std::cout << "> ";
            std::cout.write( m_response, bytesTransferred );
            std::cout << std::endl;

            m_ioService.post(
                [ this ](){ this->runImpl(); }
            );
        }
    }

private:

    enum { m_maxLength = 1024 };
    char m_response[ m_maxLength ];
};

#endif

