
all: client server

client: client.o
	clang++ --std=c++11 client.o -o client -lboost_system -lboost_thread -pthread

server: server.o connection.o
	clang++ --std=c++11 server.o connection.o -o server -lboost_system -lboost_thread -pthread

client.o: client.cpp
	clang++ --std=c++11 -g client.cpp -c -o client.o

server.o: server.cpp server.hpp connection.hpp connection_manager.hpp task.hpp
	clang++ --std=c++11 -g -c server.cpp -o server.o

connection.o: connection.cpp connection.hpp server.hpp
	clang++ --std=c++11 -g -c connection.cpp -o connection.o

.PHONY: clean
clean:
	rm *.o client server

