#ifndef _SIGN_TASK_HPP_
#define _SIGN_TASK_HPP_

#ifndef SERVER_SSL
    #error Please build with SSL=1
#endif

#include <iostream>
#include <sstream>
#include <string>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>

#include "../core/iconnection.hpp"
#include "../core/itask.hpp"

#include "./sign_connection.hpp"

struct Key
{
    RSA * m_rsa;
};

class SignTask
    : public ITask
{
public:
    SignTask()
    {
    }

    void setConnection( IConnection * connection )
    {
        m_connection = connection;
    }

    IConnection::Action getStartAction() const
    {
        return IConnection::Action::Read;
    }

    IConnection::Action parse(
        char const * const buffer,
        std::size_t const bytesTransferred
    );

    void parseError();

    void process();

private:

    Key const & getKey() const;

    void sign(
        char const * const buffer,
        std::size_t const bytesTransferred
    );

    void sendPublicKey();

private:
    IConnection * m_connection;
};

void SignTask::parseError()
{
}

void SignTask::process()
{
}

typename IConnection::Action SignTask::parse(
    char const * const buffer,
    std::size_t const bytesTransferred
)
{
    if( buffer[ 0 ] == 's' )
    {
        sign( buffer + 2, bytesTransferred - 2 );
    }
    else if( buffer[ 0 ] == 'k' )
    {
        sendPublicKey();
    }

    return IConnection::Action::Process;
}

Key const & SignTask::getKey() const
{
    auto const stateConnection = dynamic_cast< StateConnection< Key > * >( m_connection );
    return stateConnection->getState();
}

void SignTask::sign(
    char const * const buffer,
    std::size_t const bytesTransferred
)
{
    if( bytesTransferred != SHA_DIGEST_LENGTH )
    {
    }
    else
    {
        char signatureBuffer[ 1024 ];
        unsigned signatureSize = 0;

        auto const signResult = RSA_sign(
            NID_sha1,
            (unsigned char const*) buffer,
            bytesTransferred,
            (unsigned char*) signatureBuffer,
            & signatureSize,
            getKey().m_rsa
        );

        if( signResult == 1 )
        {
            m_connection->response(
                signatureBuffer,
                signatureSize
            );
        }
        else
        {
            std::string const message = "Sign error: " + std::to_string( ERR_get_error() );

            m_connection->response(
                message.c_str(),
                message.size()
            );
        }
    }
}

void SignTask::sendPublicKey()
{
    char* buffer = nullptr;
    int const size = i2d_RSAPublicKey(
        getKey().m_rsa,
        (unsigned char**)( & buffer )
    );

    m_connection->response(
        buffer,
        size
    );

    free( buffer );
}

#endif

