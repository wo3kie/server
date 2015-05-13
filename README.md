# server
C++ boost::asio based async client/server

# how to run
clang++ --std=c++11 -g client.cpp -o client -lboost_system -lboost_thread -pthread
clang++ --std=c++11 -g server.cpp -o server -lboost_system -lboost_thread -pthread

#how to test
emulate server with nc
