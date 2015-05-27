
all: client chat

client: client.o
	clang++ --std=c++11 client.o -o client -lboost_system -lboost_thread -pthread

client.o: client.cpp
	clang++ --std=c++11 -g client.cpp -c -o client.o

chat: chat.o 
	clang++ --std=c++11 chat.o -o chat -lboost_system -lboost_thread -pthread

chat.o: server.hpp server.tpp connection.hpp connection.tpp connection_manager.hpp task_chat.hpp
	clang++ --std=c++11 -g -c chat.cpp -o chat.o

.PHONY: clean
clean:
	rm *.o client chat

