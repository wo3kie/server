## Copyright (C) 2015 Lukasz Czerwinski

## server
C++ boost::asio based async client/server

## website
https://github.com/wo3kie/server

## license
For license please refer to LICENSE file.

## requirements
C++11  
g++ / clang++  
boost  
OpenSSL (optionally)

## how to build it
make  
or  
make SSL=1

## examples
Please run server and client  
* echo + client_console  
* chat + client_console  
* sign + client_sign  
There is 'readme' file in each directory. See it for more details.

## Tutorial
Please find './tutorial' directory with a detailed description how
to create your own server and how to run it.

## Content
- core - basic framework to create server, connection and task  
- examples
    - chat - server for chatting  
    - client_console - line based client, use it with: chat, echo  
    - client_sign - simple client to work with sign server. It allows you to sign a file, get public key from server and offline - verification of signature  
    - pem - directory with public/private keys and certificates  
    - sign - server for signing a message, use it with client_sign. It allows you to sign a message and get public key  
- ext - contains extension for server  
    - broadcast - send message for everyone  
    - log - print message on server's console  
    - state - make data in server shared betwee all connections  
    - unicast - first set id for a connection and then sent a message to one connection with that id  
- tutorial - an example how to create server

