# SN2_P2
Systems and networks project 2

## Overview
This program creates a basic HTTP server in C++. When the program is compiled and executed, the server is automatically created.
It then listens for incoming client connections on port 60001. It is currently capable of handling both GET and POST requests to specific html 
files that are found in the project file. Any other file attempted by the client will result in a 404 Not Found message. Program activity,
including but not limited to client requests, server responses, and errors are logged. 

## Contributors
Aydan Shankland and Kevin Albee

## How to Run the Program
1. Compile the Program
Using WSL, run the 'make' command in the project directory. Ensure the Makefile is included and properly configured. 
This will generate an executable named httpServer.

2. Running the Program
After compilation, run the server program using the following command:

./httpServer

By default, the server will listen on port 60001. You can access the server by opening a web browser and entering:

http://localhost:60001

3. Exit the Program
To stop the server, use Ctrl+C in the terminal. The server will close any open sockets. 

## Cleaning Up
To remove the compiled files, run: 'make clean'