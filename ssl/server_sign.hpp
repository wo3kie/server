#ifndef _TASK_SIGN_HPP_
#define _TASK_SIGN_HPP_

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

#include "./iconnection.hpp"

struct Key
{
    void init( int argc, char* argv[] );
    void destroy();

    RSA * m_rsa;
};

void Key::init( int argc, char* argv[] )
{
    const std::string privateKeyPemString(
        "-----BEGIN RSA PRIVATE KEY-----\n"
        "Proc-Type: 4,ENCRYPTED\n"
        "DEK-Info: DES-EDE3-CBC,D6929111D2DE4526\n"
        "\n"
        "Tv3iyRZBBlZS/20aZOODca/U5RMt+7SbuusTJEzaTXrdixiEtyKRoHeW5H8tqALW\n"
        "tBqpRtAcmgV6CxYHC8Ro2//8tXm/vVqhLVySnLbwXS5wt3JD4J6Z4hVsawc6Rj/Q\n"
        "IgOBefT0wAPchW190+bZ/wdgl2At0Z2VUoxlxvM4TdVIoXi5UTJD3Ue4lxBt++mE\n"
        "74dCoF8iR/G/coVQGCOptH351PZEIthPkUUxjBdqyXJC2ArwMgUUmHR7yr1n7Xxu\n"
        "ND3OJp6bg0fXE+TLWFZu/PUf5WVKwxfQaxyYIPKquKdhle45SIr52qH+XlBOAEdg\n"
        "pn83c6ZD4T1zxolFUARZyianbXja5hrCfUzdBFaRXOLqdhN/zIw5l61tzNayuu18\n"
        "6w3SwgXgbQP3bP816Uz9mPg+NRUEVbFbRdEabzBN436HaqUtKYCqVF+qHbSItlwk\n"
        "A+OfJIxZXMbb2I9HKUK1T6Wp08fGGAD7HqeaVtPfXNtDvncg7ZVHB8OTaI41AcXQ\n"
        "/PP0juscua0H2eMBkPg3tLXjsWCXiknsFCYrRuQOMSbXzU8bCE6E37YascIpveNh\n"
        "sNu0sd0rJyCn/ul1vXEA2XE82f1wXBnOtx0266nCk3Ox8d5wA+LjuAxLVFGtaNXj\n"
        "ozVvMUwA3jlDutz7ZJxUjRKWyR2tgzRboeqkErdmWHDhOt8dA9vodtgQ7GugRZd0\n"
        "/qq7j9Dq5PazLJx9RYaTQiREDbCx4Ur8YeE9zt8/uzAsttOQQSvoixJDWmrbpUD8\n"
        "V3XBzhjQC4q5WspIW7N+xJWeyNfed2jx8VpMKFkbqKA=\n"
        "-----END RSA PRIVATE KEY-----\n"
    );

    auto* const bioBuffer = BIO_new_mem_buf( 
        (void*) privateKeyPemString.c_str(),
        (int) privateKeyPemString.size()
    );

    m_rsa = PEM_read_bio_RSAPrivateKey(
        bioBuffer,
        nullptr,
        nullptr,
        (void*) "1234" /* password */
    );
}

void Key::destroy()
{
}

class TaskSign
{
public:
    TaskSign( IConnection * connection )
        : m_connection( connection )
    {
    }

    static IConnection::Action start()
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

    Key const * getKey() const
    {
        return static_cast< Key const * >( m_connection->getState() );
    }

    void sign(
        char const * const buffer,
        std::size_t const bytesTransferred
    );

    void sendPublicKey();

private:
    IConnection * m_connection;
};

typename IConnection::Action TaskSign::parse(
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

void TaskSign::sign(
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
            getKey()->m_rsa
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

void TaskSign::sendPublicKey()
{
    char* buffer = nullptr;
    int const size = i2d_RSAPublicKey(
        getKey()->m_rsa,
        (unsigned char**)( & buffer )
    );

    m_connection->response(
        buffer,
        size
    );

    free( buffer );
}

void TaskSign::parseError()
{
}

void TaskSign::process()
{
}

#endif

