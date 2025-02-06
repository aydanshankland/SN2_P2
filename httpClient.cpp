/**
 * This program creates the TCP client in C++.
 *
 * @author Aydan Shankland & Kevin Albee
 * @date 2/4/2025
 * @info Course COP4635
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <cstring>
#include <sys/socket.h> //API and definitions for the sockets
#include <netinet/in.h> //Structures to store address information
#include <unistd.h>     // For read, close

#define BUFFER_SIZE 1024


int main()
{
    const int portNum = 60001;
    char buffer[BUFFER_SIZE] = {0};

    std::cout << "Client starting." << std::endl << std::endl;

    // Create the socket
    // Referenced: Dr. Mishra's tcpClient3.c file, as well as https://www.geeksforgeeks.org/socket-programming-in-cpp/
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // check if server socket was made correctly
    if (clientSocket == -1)
    {
        perror("Client failed to create a socket.");
        exit(1);
    }

    // define the server address
    sockaddr_in servAddress{};                  // declaring a structure for the address
    servAddress.sin_family = AF_INET;         // Structure Fields' definition: Sets the address family of the address the client would connect to
    servAddress.sin_port = htons(portNum);    // Passing the port number - converting in right network byte order
    servAddress.sin_addr.s_addr = INADDR_ANY; // Connecting to 0.0.0.0

    // connecting the socket to the IP address and port
    // Params: which socket, cast for server address, its size
    if (connect(clientSocket, (struct sockaddr *)&servAddress, sizeof(servAddress)) == -1)
    {
        perror("Connection failed.");
        close(clientSocket);
        exit(1);
    }

    std::cout << "Client connected to server." << std::endl;

    std::string fileName = "";
    // connection loop with server
    while (true)
    {
        std::cout << "Please enter the file name or 'exit' to quit." << std::endl;
        std::getline(std::cin, fileName);

        if(fileName == "exit")
        {
            break;
        }

        std::ostringstream reqStream;
            reqStream << "GET /" << fileName << "HTTP/1.1\r\n"
                        << "host: localhost\r\n"
                        << "Connection: close\r\n"
                        << "\r\n";
                          
        std::string httpReq = reqStream.str();

        send(clientSocket, httpReq.c_str(), httpReq.size(), 0);

        // std::string clientMsg = "message=Client+Started"; 
        // std::string httpReq = 
        //     "POST / HTTP/1.1\r\n"
        //     "host: localhost\r\n"
        //     "Content-Type: application/x-www-form-urlencoded\r\n"
        //     "Content-Length: " +
        //     std::to_string(clientMsg.length()) + "\r\n\r\n" + 
        //     clientMsg;

        // send(clientSocket, httpReq.c_str(), httpReq.size(), 0);

        // Receive the HTTP response from server
        int bytesRecvd = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRecvd < 0)
        {
            perror("Read from server failed");
            break;
        }

        buffer[bytesRecvd] = '\0';
        
    }

    // close the server socket
    close(clientSocket);

    std::cout << "Client side socket closed." << std::endl
              << std::endl;

    return 0;
}


