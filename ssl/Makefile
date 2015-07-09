include ../common.mk

CXXFLAGS+= -DSERVER_SSL -I..
LDLIBS+=-lssl -lcrypto

SRCS=$(shell ls ../*.cpp)
SRCS_SSL=$(shell ls *.cpp)

HDRS=$(shell ls ../*.hpp ../*.tpp)
HDRS_SSL=$(shell ls *.hpp)

OBJS_SSL=$(subst .cpp,_ssl.o,$(SRCS))
OBJS_SSL:=$(subst ../,,$(OBJS_SSL))
OBJS_SSL+=$(subst .cpp,_ssl.o,$(SRCS_SSL))

APPS_SSL=$(subst .cpp,_ssl,$(SRCS))
APPS_SSL:=$(subst ../,,$(APPS_SSL))
APPS_SSL+=$(subst .cpp,_ssl,$(SRCS_SSL))

.PRECIOUS: %_ssl.o

.PHONY: all
all: $(APPS_SSL)

%_ssl: %_ssl.o
	$(CXX) $(LDLIBS) $< -o $@

%_ssl.o: ../%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%_ssl.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS_SSL) $(APPS_SSL)

.PHONY: pem
pem:
	openssl genrsa 2048 > ca-key.pem
	openssl req -new -x509 -nodes -days 365 -key ca-key.pem -out ca.pem
	openssl req -newkey rsa:2048 -nodes -days 365 -keyout server-key.pem -out server-req.pem
	openssl rsa -in server-key.pem -out server-key.pem
	openssl x509 -req -in server-req.pem -days 365 -CA ca.pem -CAkey ca-key.pem -set_serial 01 -out server-cert.pem
	cat server-cert.pem ca.pem > server.pem

.PHONY: depend
depend: $(SRCS_SSL) $(SRCS)
	$(CXX) $(CXXFLAGS) -MM $^ > .makefile.ssl.dep
	sed -i 's/\.o:/_ssl\.o:/' .makefile.ssl.dep

include .makefile.ssl.dep

