include ../common.mk

SRCS=$(shell ls *.cpp)
HDRS=$(shell ls *.hpp)
OBJS=$(subst .cpp,.o,$(SRCS))
APPS=$(subst .cpp,,$(SRCS))

.PHONY: all
all: $(APPS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(APPS)

.PHONY: depend
depend: $(SRCS_SSL) $(SRCS)
	$(CXX) $(CXXFLAGS) -MM $^ > .makefile.dep

include .makefile.dep

