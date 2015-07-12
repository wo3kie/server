#include "../core/server.hpp"
 
#include "../ext/state/connection.tpp"
 
#include "./server.hpp"
#include "./task.hpp"

int main( int argc, char* argv[] )
{
    SignServer< Key >( argv[1] ).run();
}

