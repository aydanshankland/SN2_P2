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

std::string readFile(const std::string &fileName);
std::string readMessage(const std::string buffer);
void sendMsg(int clientMsg);

int main()
{
    const int portNum = 60001;
    char buffer[BUFFER_SIZE] = {0};

    std::cout << "Client starting." << std::endl
              << std::endl;

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
    sockaddr_in servAddress;                  // declaring a structure for the address
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

    std::string clientMsg = ""; 

    send(clientSocket, clientMsg.c_str(), clientMsg.size(), 0);
    sleep(5);
    // connection loop with server
    // while (1)
    // {
    //     sendMsg(clientSocket);
    // }

    // close the server socket
    close(clientSocket);

    // std::cout << "Client socket closed." << std::endl
    //           << std::endl;

    return 0;
}


void sendMsg(int socket){
    std::string message;
    std::cout << "Enter message to send: ";
    std::getline(std::cin, message);

    send(socket, message.c_str(), message.size(), 0);

    char buffer[BUFFER_SIZE] = {0};

}

