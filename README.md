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

## Tutorial
Please find './tutorial/server' directory with a detailed description how to create your  
own server and how to run it.

Inside './tutorial/plugins' you can find a description how to create your own server's  
plugin.

## Content
- core - basic framework to create server, connection and task  
- example
    - chat - server for chatting  
    - client_console - line based client, use it together with: chat, echo  
    - echo - simple server for echo  
- plugin - contains plugins for a server  
    - broadcast - send a message for everyone  
    - log - print a message on server's console  
    - state - make data in server shared between all connections  
    - unicast - initially set an id for a connection and then sent a message to a connection identifying it by id  
- tutorial - documentation
    - server - tutotial how to create your server
    - plugin - tutorial how to extend server's functionality

