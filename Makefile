include ./common.mk

APPS=client_console chat echo

ifeq ($(SSL),1)
APPS+=client_sign sign
endif

.PHONY: all
all:
	for d in $(APPS);\
	do                      \
		$(MAKE) $@ -C $$d;  \
	done

.PHONY: clean
clean:
	for d in $(APPS);\
	do                      \
		$(MAKE) $@ -C $$d;  \
	done

.PHONY: pem
pem:
	$(MAKE) $@ -C pem

.PHONY: depend
depend:
	for d in $(APPS);\
	do                      \
		$(MAKE) $@ -C $$d;  \
	done

