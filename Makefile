
all: client chat

client: client.o
	clang++ --std=c++11 client.o -o client -lboost_system -lboost_thread -pthread

chat: chat.o server.o connection.o
	clang++ --std=c++11 chat.o server.o connection.o -o chat -lboost_system -lboost_thread -pthread

client.o: client.cpp
	clang++ --std=c++11 -g client.cpp -c -o client.o

server.o: server.cpp server.hpp connection.hpp connection_manager.hpp task_chat.hpp
	clang++ --std=c++11 -g -c server.cpp -o server.o

connection.o: connection.cpp connection.hpp server.hpp
	clang++ --std=c++11 -g -c connection.cpp -o connection.o

chat.o: server.hpp connection.hpp connection_manager.hpp task_chat.hpp
	clang++ --std=c++11 -g -c chat.cpp -o chat.o

.PHONY: clean
clean:
	rm *.o client chat

