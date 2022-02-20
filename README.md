# Simple Webserver

## Prerequisites

---
This server can only be build and used on a Linux-based operating system.

- **The GCC compiler**. Used to compile the project.
- **OpenSSL** for SSL support.
- **ZLib**, used for gzip compression.

<br>

## Build Instructions

---
Run the following commands in the terminal:

    > git clone https://github.com/R0b4/Server
    > cd Server
    > make clean; make

After this the server executable is found in **`Server/bin/main`**.  

<br>

## Build Instructions for dynamic pages

---

Dynamic pages have to be programmed in to the webserver. To do this you can follow the same instructions as the above steps.  

After this you will have to write the source code for your dynamic pages. To do this you will have to implement the functions, found in the header file **`Server/include/server/dynamic.hpp`**.  

You can use the code in **`Server/dynamic/dynamic.cpp`** as a template for this.  

After you wrote your own code for dynamic pages. You will have to compile it. For simplicity's sake let's say you have one file called **`mydynamic.cpp`**, then follow these instructions to make a webserver out of it:

    > g++ -o mydynamic -c mydynamic.cpp -O2 -Iopenssl/openssl-*/include -Iinclude
    > g++ -o myserver bin/obj mydynamic -Lopenssl/openssl-* -lssl -lcrypto -lz -O2

The executable will then be called **`myserver`**.

<br>

## Usage

---

    server COMMAND ARGS

Commands:  

- run
  - Takes one argument, namely the directory to the website.
  - Used to run the server for a certain website.
- config
  - Takes one argument, namely the directory to the website.
  - Opens a command line interface to used to change settings for the server.
- create
  - Takes one argument, namely the directory to where you want the website to be.
  - creates a directory structure for a new website.
- gzip
  - Takes one argument, namely the directory to the website.
  - gzips all pages that you specified in the settings. You need to run this anytime, you change any of these pages or add a new one.