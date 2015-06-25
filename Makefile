CXX=clang++
CXXFLAGS=--std=c++11 -g
LIBS=-lboost_system -lboost_thread -pthread

ifeq ($(SSL), 1)
CXXFLAGS += -DSERVER_SSL
LIBS += -lssl -lcrypto
endif

SRCS=$(shell ls *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))
APPS=$(subst .cpp,,$(SRCS))

all: depend apps

depend: $(SRCS)
	$(CXX) -MM $^ > .makefile.dep

apps: $(APPS)

client_line: client_line.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) $< -o $@

chat: chat.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) $< -o $@

echo: echo.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) $< -o $@

day_time: day_time.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) $< -o $@

pem:
	openssl genrsa 2048 > ca-key.pem
	openssl req -new -x509 -nodes -days 365 -key ca-key.pem -out ca.pem
	openssl req -newkey rsa:2048 -nodes -days 365 -keyout server-key.pem -out server-req.pem
	openssl rsa -in server-key.pem -out server-key.pem
	openssl x509 -req -in server-req.pem -days 365 -CA ca.pem -CAkey ca-key.pem -set_serial 01 -out server-cert.pem
	cat server-cert.pem ca.pem > server.pem

.PHONY: clean
clean:
	rm -f *.o .makefile.dep $(APPS)

#include .makefile.dep

