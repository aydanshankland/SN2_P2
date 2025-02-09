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
#include <arpa/inet.h>   // For inet_pton()

#define BUFFER_SIZE 1024

int main()
{
    const int portNum = 60001;
    char buffer[BUFFER_SIZE] = {0};

    std::cout << "Client starting." << std::endl
              << std::endl;

    std::string fileName = "";
    // connection loop with server
    while (true)
    {
        std::string inputLine, serverAddress;
        std::cout << "Please enter the server address and file name or 'exit' to quit." << std::endl;
        std::cout << "Example: \n" << "127.0.0.1 index.html\n\n";
        std::getline(std::cin, inputLine);

        if(inputLine == "exit") {
            break;
        }

        std::istringstream iss(inputLine);  // Split the input
        iss >> serverAddress;               // First word = server address
        std::getline(iss, fileName);        // Rest = file name
        fileName.erase(0,1);                // remove the space from the front of the file name


        // Create the socket
        // Referenced: Dr. Mishra's tcpClient3.c file, as well as https://www.geeksforgeeks.org/socket-programming-in-cpp/
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        // check if server socket was made correctly
        if (clientSocket == -1)
        {
            perror("Client failed to create a socket.");
            continue;
        }

        // define the server address
        sockaddr_in servAddress{};             // declaring a structure for the address
        servAddress.sin_family = AF_INET;      // Structure Fields' definition: Sets the address family of the address the client would connect to
        servAddress.sin_port = htons(portNum); // Passing the port number - converting in right network byte order

        // Convert the user-inputted IP address from string to binary
        if (inet_pton(AF_INET, serverAddress.c_str(), &servAddress.sin_addr) <= 0)
        {
            std::cout << "Invalid address/Address not supported.\n\n";
            close(clientSocket);
            continue; // Skip and allow the user to enter another request
        }

        // connecting the socket to the IP address and port
        // Params: which socket, cast for server address, its size
        if (connect(clientSocket, (struct sockaddr *)&servAddress, sizeof(servAddress)) == -1)
        {
            std::cout << "Server did not respond...\n\n"; 
            close(clientSocket);
            continue;
        }

        std::ostringstream reqStream;

        reqStream << "GET /" << fileName << " HTTP/1.1\r\n"
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

        std::cout << buffer << std::endl;

        // close the server socket
        close(clientSocket);
    }

    std::cout << "Shutting down client. \n";

    return 0;
}
