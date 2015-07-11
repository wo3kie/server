include ../common.mk

.PHONY: all
all:

.PHONY: pem
pem:
	openssl genrsa 2048 > ca-key.pem
	openssl req -new -x509 -nodes -days 365 -key ca-key.pem -out ca.pem
	openssl req -newkey rsa:2048 -nodes -days 365 -keyout server-key.pem -out server-req.pem
	openssl rsa -in server-key.pem -out server-key.pem
	openssl x509 -req -in server-req.pem -days 365 -CA ca.pem -CAkey ca-key.pem -set_serial 01 -out server-cert.pem
	cat server-cert.pem ca.pem > server.pem

.PHONY: clean
clean:
	rm *pem

