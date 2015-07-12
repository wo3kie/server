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
Each directory has 'readme' file for more details.

## .tpp file extension in vim
Insert line below into ~/.vimrc  
`autocmd BufNewFile,BufReadPost *.ino,*.pde set filetype=cpp`

