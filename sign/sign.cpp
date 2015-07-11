#include "../core/server.hpp"
 
#include "../ext/state_connection.tpp"
 
#include "./sign_server.hpp"
#include "./sign_task.hpp"

int main( int argc, char* argv[] )
{
    SignServer< Key >( argv[1] ).run();
}

