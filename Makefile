CC=clang++
CXX=clang++
CXXFLAGS=--std=c++11 -g
LDLIBS=-lboost_system -lboost_thread -pthread

ifeq ($(SSL), 1)
CXXFLAGS += -DSERVER_SSL
LDLIBS += -lssl -lcrypto
endif

SRCS=$(shell ls *.cpp)
HDRS=$(shell ls *.hpp *.tpp)
OBJS=$(subst .cpp,.o,$(SRCS))
APPS=$(subst .cpp,,$(SRCS))

all: $(APPS)

client_console: client_console.o

chat: chat.o

echo: echo.o

day_time: day_time.o

%.o: %.cpp %.hpp %.tpp
	$(CXX) $(CXXFLAGS) $* -c -o $@

-include .makefile.dep

depend: $(SRCS) $(HDRS)
	$(CXX) -MM $^ > .makefile.dep

pem:
	openssl genrsa 2048 > ca-key.pem
	openssl req -new -x509 -nodes -days 365 -key ca-key.pem -out ca.pem
	openssl req -newkey rsa:2048 -nodes -days 365 -keyout server-key.pem -out server-req.pem
	openssl rsa -in server-key.pem -out server-key.pem
	openssl x509 -req -in server-req.pem -days 365 -CA ca.pem -CAkey ca-key.pem -set_serial 01 -out server-cert.pem
	cat server-cert.pem ca.pem > server.pem

.PHONY: clean
clean:
	rm -f *.o $(APPS)

#include .makefile.dep

