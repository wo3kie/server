#include "./server.hpp"

#include "./task_day_time.hpp"

int main( int argc, char* argv[] )
{
    Server< TaskDayTime >( argv[1] ).run();
}

