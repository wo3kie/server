#include "../core/server.hpp"

#include "../ext/unicast_connection.tpp"
#include "../ext/log_connection.tpp"
#include "../ext/broadcast_connection.tpp"

#include "./chat_server.hpp"
#include "./chat_task.hpp"

int main( int argc, char* argv[] )
{
    ChatServer( argv[ 1 ] ).run();
}

