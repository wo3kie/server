#include "./server.hpp"

#include "./task_chat.hpp"

int main( int argc, char* argv[] )
{
    Server< TaskChat >( argv[1] ).run();
}

