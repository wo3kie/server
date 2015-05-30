CXX=clang++
CXXFLAGS=--std=c++11
LIBS=-lboost_system -lboost_thread -pthread

SRCS=$(shell ls *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))
APPS=$(subst .cpp,,$(SRCS))

all: depend apps
	
depend: $(SRCS)
	$(CXX) -MM $^ >> .makefile.dep

apps: $(APPS)

client: client.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) $< -o $@

chat: chat.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) $< -o $@

echo: echo.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) $< -o $@

day_time: day_time.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o .makefile.dep $(APPS)

#include .makefile.dep

