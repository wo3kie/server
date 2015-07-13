include ./common.mk

APPS=example/client_console example/chat example/echo

ifeq ($(SSL),1)
	APPS+=example/client_sign example/sign
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
	$(MAKE) $@ -C example/pem

.PHONY: depend
depend:
	for d in $(APPS);\
	do\
		$(MAKE) $@ -C $$d;\
	done

