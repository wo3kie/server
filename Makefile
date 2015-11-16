include ./common.mk

APPS=example/client_console example/chat example/echo example/sign example/client_sign

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

