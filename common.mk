ERR=$(shell which clang++ >/dev/null; echo $$?)
ifeq "$(ERR)" "0"
	CXX=clang++
else
	CXX=g++
endif

CC=$(CXX)
CXXFLAGS=--std=c++11 -g
LDLIBS=-lboost_system -lboost_thread -pthread

ifeq ($(SSL),1)
	CXXFLAGS+=-DSERVER_SSL
	LDLIBS+=-lssl -lcrypto
endif

