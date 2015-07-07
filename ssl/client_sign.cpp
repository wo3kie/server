#include "./client.hpp"

#include "./client_sign.hpp"
#include "./client_key.hpp"

int main( int argc, char* argv[] )
{
    if( argv[ 1 ] == std::string( "--sign" ) )
    {
        boost::asio::io_service ioService;

        const auto host = "127.0.0.1";
        const auto port = argv[ 2 ];

        Client< ReaderSign, WriterSign >( ioService, host, port, argc, argv );
    }
    else if( argv[ 1 ] == std::string( "--key" ) )
    {
        boost::asio::io_service ioService;

        const auto host = "127.0.0.1";
        const auto port = argv[ 2 ];

        Client< ReaderKey, WriterKey >( ioService, host, port );
    }
    else if( argv[ 1 ] == std::string( "--verify" ) )
    {
        char sha1[ 1024 ];

        {
            SHA_CTX ctx;
            SHA1_Init( & ctx );

            char buffer[ 4 * 1024 ];
            std::ifstream fileIn( argv[ 2 ], std::ios_base::binary );
            
            while( fileIn.read( & buffer[ 0 ], sizeof( buffer ) ) )
            {
                SHA1_Update( & ctx, & buffer[ 0 ], fileIn.gcount() );
            }
            
            SHA1_Final( (unsigned char*)( & sha1[ 0 ] ), & ctx );
        }

        RSA * rsa = nullptr;

        {
            std::ifstream keyFile( argv[ 3 ], std::ios_base::binary );
            char key[ 1024 ];
            
            keyFile.read( key, 1024 );
            unsigned keySize = keyFile.gcount();
            
            unsigned char const * buffer = (unsigned char const*)( & key[ 0 ] );
            rsa = d2i_RSAPublicKey( nullptr, & buffer, keySize );
        }

        std::ifstream signatureFile( argv[ 4 ], std::ios_base::binary );
        char signature[ 1024 ];
        signatureFile.read( signature, 1024 );
        unsigned signatureSize = signatureFile.gcount();

        const auto verifyResult = RSA_verify(
            NID_sha1,
            (unsigned char const*)( & sha1 ),
            SHA_DIGEST_LENGTH,
            (unsigned char const*) signature,
            signatureSize,
            rsa
        );

        if( verifyResult )
        {
            std::cout << "OK" << std::endl;
        }
        else
        {
            std::cout << "Failed" << std::endl;
        }

        return ! verifyResult;
    }
    else
    {
        std::cout
            << "Usage: " << argv[ 0 ] << '\n'
            << "  --sign <port> <filename>\n"
            << "  --key <port>\n"
            << "  --verify <messagefile> <keyfile> <signaturefile>\n";

        return 1;
    }

    return 0;
}

