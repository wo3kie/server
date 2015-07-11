CC=clang++
CXX=clang++
CXXFLAGS=--std=c++11 -g
LDLIBS=-lboost_system -lboost_thread -pthread

ifeq ($(SSL),1)
CXXFLAGS+=-DSERVER_SSL
LDLIBS+=-lssl -lcrypto
endif

