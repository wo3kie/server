include ./common.mk

.PHONY: all
all:
	make $@ -C client_console
	make $@ -C chat
	make $@ -C echo

.PHONY: clean
clean:
	make $@ -C client_console
	make $@ -C chat
	make $@ -C echo

.PHONY: pem
pem:
	make $@ -C pem

.PHONY: depend
depend:
	make $@ -C client_console
	make $@ -C chat
	make $@ -C echo

include .makefile.dep

