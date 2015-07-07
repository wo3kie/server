include ./common.mk

SRCS=$(shell ls *.cpp)
HDRS=$(shell ls *.hpp *.tpp)
OBJS=$(subst .cpp,.o,$(SRCS))
APPS=$(subst .cpp,,$(SRCS))

.PHONY: all
all: $(APPS)
ifeq ($(SSL),1)
	make all -C ssl
endif

.PHONY: clean
clean:
	rm -f $(OBJS) $(APPS)
ifeq ($(SSL),1)
	make clean -C ssl
endif

.PHONY: pem
pem:
	make pem -C ssl

.PHONY: depend
depend: $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) -MM $^ > .makefile.dep
ifeq ($(SSL),1)
	make depend -C ssl
endif

include .makefile.dep

