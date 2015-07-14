include ./common.mk

APPS=example/client_console example/chat example/echo

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

.PHONY: depend
depend:
	for d in $(APPS);\
	do\
		$(MAKE) $@ -C $$d;\
	done

