#ifndef _SIGN_SERVER_HPP_
#define _SIGN_SERVER_HPP_

#include "../ext/state/server.hpp"

#include "./task.hpp"
#include "./connection.hpp"

template<
    typename TState
>
struct SignServer
    : public StateServer< TState >
{
    SignServer( std::string const & port )
        : Server( port )
        , StateServer< TState >( port )
    {
    }

    ITaskPtr createTask() override
    {
        return ITaskPtr( new SignTask() );
    }

    IConnectionPtr createConnection() override
    {
        return IConnectionPtr( new SignConnection< TState >( StateServer< TState >::m_ioService, this, createTask() ) );
    }

    void run() override
    {
        loadKey();
        Server::run();
    }

private:

    void loadKey()
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

        StateServer< TState >::m_state.m_rsa = PEM_read_bio_RSAPrivateKey(
            bioBuffer,
            nullptr,
            nullptr,
            (void*) "1234" /* password */
        );
    }
};

#endif

