include ./common.mk

APPS=examples/client_console examples/chat examples/echo

ifeq ($(SSL),1)
APPS+=examples/client_sign examples/sign
endif

.PHONY: all
all:
	for d in $(APPS);\
	do\
		$(MAKE) $@ -C $$d;\
	done

.PHONY: clean
clean:
	for d in $(APPS);\
	do\
		$(MAKE) $@ -C $$d;\
	done

.PHONY: pem
pem:
	$(MAKE) $@ -C examples/pem

.PHONY: depend
depend:
	for d in $(APPS);\
	do\
		$(MAKE) $@ -C $$d;\
	done

