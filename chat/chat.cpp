#include "../core/server.hpp"

#include "../ext/unicast/connection.tpp"
#include "../ext/log/connection.tpp"
#include "../ext/broadcast/connection.tpp"

#include "./chat_server.hpp"
#include "./chat_task.hpp"

int main( int argc, char* argv[] )
{
    ChatServer( argv[ 1 ] ).run();
}

