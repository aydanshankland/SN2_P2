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
    std::string loopPrintLinesH1 = "================================================================";
    std::string loopPrintLinesH2 = "----------------------------------------------------------------";

    std::cout << "Client starting." << std::endl
              << std::endl;

    std::string fileName = "";
    // connection loop with server
    while (true)
    {
        std::cout << loopPrintLinesH1 << std::endl;
        std::string inputLine, serverAddress;
        std::cout << "Please enter the server address and file name or 'exit' to quit." << std::endl;
        std::cout << "Example:  " << "127.0.0.1 index.html\n\n";
        std::cout << "To send a message instead, preceed your input with: msg:" << std::endl;
        std::cout << "Example:  " << "msg: Hello World!\n\n";
        std::cout << loopPrintLinesH2 << std::endl;
        std::getline(std::cin, inputLine);

        if(inputLine == "exit") {
            break;
        }

        std::cout << "Client input: " << inputLine << "\n" << std::endl;
        std::string firstWord = "";
        std::string msgBody = "";

        std::istringstream iss(inputLine);  // Split the input
        iss >> firstWord;               // First word = server address

        if(firstWord != "msg:")
        {
            serverAddress = firstWord;
            std::getline(iss, fileName);        // Rest = file name
            fileName.erase(0,1);                // remove the space from the front of the file name

            // Validate that the input has both an IP entry and file entry
            if(serverAddress.empty() || fileName.empty())
            {
                std::cout << "ERROR: Invalid entry. Expecting an IP address AND a file name." << std::endl;
                continue;
            }   
        }else{
            msgBody = inputLine.substr(4); // Extract message after "msg:"
            std::replace(msgBody.begin(), msgBody.end(), ' ', '+'); // Convert spaces to +
            std::cout << "msgBody: " << msgBody << std::endl;
        }

        // Create the socket
        // Referenced: Dr. Mishra's tcpClient3.c file, as well as https://www.geeksforgeeks.org/socket-programming-in-cpp/
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        // check if server socket was made correctly
        if (clientSocket == -1)
        {
            perror("Error: Client failed to create a socket.");
            continue;
        }

        // define the server address
        sockaddr_in servAddress{};             // declaring a structure for the address
        servAddress.sin_family = AF_INET;      // Structure Fields' definition: Sets the address family of the address the client would connect to
        servAddress.sin_port = htons(portNum); // Passing the port number - converting in right network byte order

        // If a client is sending a message, then the serverAddress will still be blank and this IP needs to be added.
        if(serverAddress.empty() || serverAddress == "")
        {
            serverAddress = "127.0.0.1";
        }

        // Convert the user-inputted IP address from string to binary
        if (inet_pton(AF_INET, serverAddress.c_str(), &servAddress.sin_addr) <= 0)
        {
            std::cout << "ERROR: Invalid address/Address not supported.\n\n";
            close(clientSocket);
            continue; // Skip and allow the user to enter another request
        }

        // Needs a timeout, otherwise the client will hang indefinitely when a valid yet unfindable IP is entered. 
        // ref1: https://man.freebsd.org/cgi/man.cgi?query=setsockopt&sektion=2
        // ref2: https://stackoverflow.com/questions/4181784/how-to-set-socket-timeout-in-c-when-making-multiple-connections
        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        // Sets the send timeout
        if(setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)))
        {
            perror("ERROR: Send timeout failed.");
        }

        // Sets the receive timeout
        if(setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)))
        {
            perror("ERROR: Receive timeout failed.");
        }

        // connecting the socket to the IP address and port
        // Params: which socket, cast for server address, its size
        if (connect(clientSocket, (struct sockaddr *)&servAddress, sizeof(servAddress)) == -1)
        {
            perror("ERROR: Server connection failed.");
            std::cout << "Error: No response from server at " << serverAddress << "\n\n"; 
            close(clientSocket);
            continue;
        }

        std::ostringstream reqStream;

        if(msgBody.empty() || msgBody == "")
        {
            reqStream << "GET /" << fileName << " HTTP/1.1\r\n"
                    << "Host: localhost\r\n"
                    << "Connection: close\r\n"
                    << "\r\n";
        }else{
            reqStream << "POST / HTTP/1.1\r\n"
                    << "Host: localhost\r\n"
                    << "Content-Length: " << msgBody.length() << "\r\n"
                    << "Content-Type: application/x-www-form-urlencoded\r\n\r\n"
                    << "message=" << msgBody;
        }

        std::string httpReq = reqStream.str();

        send(clientSocket, httpReq.c_str(), httpReq.size(), 0);

        std::cout << "Client request: \n" << httpReq << std::endl;

        // Receive the HTTP response from server
        int bytesRecvd = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRecvd == 0)
        {
            std::cout << "Server response: connection closed." << std::endl;
            break;
        }else if (bytesRecvd < 0)
        {
            std::cout << "Server response: no response\nRead from server failed" << std::endl;
            perror("Server response: no response\nRead from server failed");
            break;
        }

        buffer[bytesRecvd] = '\0';

        std::cout << "\nServer Response: \n" << buffer << std::endl;

        // close the server socket
        close(clientSocket);
    }

    std::cout << "Client socket closed. \n";

    return 0;
}
