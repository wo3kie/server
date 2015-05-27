
all: client chat echo

client: client.cpp
	clang++ --std=c++11 -g client.cpp -o client -lboost_system -lboost_thread -pthread

chat: server.hpp server.tpp connection.hpp connection.tpp connection_manager.hpp task_chat.hpp
	clang++ --std=c++11 -g chat.cpp -o chat -lboost_system -lboost_thread -pthread

echo: server.hpp server.tpp connection.hpp connection.tpp connection_manager.hpp task_echo.hpp
	clang++ --std=c++11 -g echo.cpp -o echo -lboost_system -lboost_thread -pthread

.PHONY: clean
clean:
	rm *.o client chat echo

