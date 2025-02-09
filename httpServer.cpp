/**
 * This program creates the TCP server in C++.
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
#include <thread> //https://en.cppreference.com/w/cpp/thread/thread
#include <atomic>
#include <algorithm> // For transforming input to lowercase
#include <cctype>


#define BUFFER_SIZE 1024

std::atomic<bool> serverClosed(false); // https://en.cppreference.com/w/cpp/atomic/atomic
std::string readFile(const std::string &fileName);
void exitServer();

int main()
{
    const int portNum = 60001;
    char buffer[BUFFER_SIZE] = {0};

    std::cout << "Server starting." << std::endl
              << std::endl;

    // Create the socket
    // Reference: Dr. Mishra's tcpServer3.c file, as well as https://www.geeksforgeeks.org/how-to-read-from-a-file-in-cpp/
    int servSocket = socket(AF_INET, SOCK_STREAM, 0);

    // check if server socket was made correctly
    if (servSocket == -1)
    {
        perror("Server failed to create a socket.");
        exit(1);
    }

    // Opens ports for reuse immediately after shutdown
    int opt = 1;
    setsockopt(servSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //https://www.ibm.com/docs/en/zos/2.4.0?topic=calls-setsockopt

    // define the server address
    sockaddr_in servAddress{};                  // declaring a structure for the address
    servAddress.sin_family = AF_INET;         // Structure Fields' definition: Sets the address family of the address the client would connect to
    servAddress.sin_port = htons(portNum);    // Passing the port number - converting in right network byte order
    servAddress.sin_addr.s_addr = INADDR_ANY; // Connecting to 0.0.0.0

    // binding the socket to the IP address and port
    // Params: which socket, cast for server address, its size
    if (bind(servSocket, (struct sockaddr *)&servAddress, sizeof(servAddress)) == -1)
    {
        perror("Bind failed.");
        close(servSocket);
        exit(1);
    }else{
        std::cout << "Server address: " << servSocket << std:: endl;
    }

    // listen for simultaneous connections
    // which socket, how many connections
    if (listen(servSocket, 5) == -1)
    {
        perror("Listen failed.");
        close(servSocket);
        exit(1);
    }

    std::cout << "Server listening on port " << portNum << "..." << std::endl
              << std::endl;
    
    //Open thread
    std::thread t(exitServer);

    // accept client connections
    while (!serverClosed)
    {
        int clientSocket = accept(servSocket, nullptr, nullptr); // server socket to interact with client, structure like before - if you know - else nullptr for local connection

        if(serverClosed)
        {
            close(clientSocket);
            break;
        }

        if (clientSocket < 0)
        {
            perror("Failed to accept client.");
            continue;
        }

        // Receive the HTTP request
        int bytesRecvd = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRecvd < 0)
        {
            perror("Read failed");
            close(clientSocket);
            continue;
        }

        buffer[bytesRecvd] = '\0';

        // get the file name from the http request
        std::istringstream httpReqStream(buffer);
        std::string method, path, line;
        httpReqStream >> method >> path;

        std::cout << "Request Method: " << method << std::endl;
        std::cout << "Request Path: " << path << std::endl << std::endl;

        // check method type
        if (method == "POST")
        {
            // retrieve message from client
            std::string headers;
            while (std::getline(httpReqStream, line) && line != "\r")
            {
                headers += line + "\n";
            }

            // Read the request body (message payload)
            std::string message;
            std::getline(httpReqStream, message);

            // replace + with spaces
            std::replace(message.begin(), message.end(), '+', ' ');
            // remvove "message="
            message.erase(0, 8);
            std::cout << "Message from client: " << message << std::endl;

            if(message == "stop")
            {
                serverClosed = true;
                std::cout << "Received 'stop' command from client. Server shutting down." << std::endl;
            }

            //redirect them back to the main screen
            path = "index.html";
            std::string fileContent = readFile(path);
            std::ostringstream resStream;
                resStream << "HTTP/1.1 200 OK\r\n"
                          << "Content-Type: text/html\r\n"
                          << "Content-Length: " << fileContent.size() << "\r\n\r\n"
                          << fileContent;
                std::string httpRes = resStream.str();
                send(clientSocket, httpRes.c_str(), httpRes.size(), 0);
                std::cout << "Server response: " << httpRes << std::endl;
        }
        else if (!path.empty() && path != "")
        {

            // remove "/" from file path
            if (path[0] == '/')
            {
                path = path.substr(1);
            }

            // if no file path is designated we just take them to index.html
            if (path.empty())
            {
                path = "index.html";
            }

            std::string fileContent = readFile(path);

            if (!fileContent.empty())
            {
                std::ostringstream resStream;
                resStream << "HTTP/1.1 200 OK\r\n"
                          << "Content-Type: text/html\r\n"
                          << "Content-Length: " << fileContent.size() << "\r\n\r\n"
                          << fileContent;
                std::string httpRes = resStream.str();
                send(clientSocket, httpRes.c_str(), httpRes.size(), 0);
               // std::cout << "Server response: " << httpRes << std::endl;
            }
            else // file path choosen is not valid, send "404 not found"
            {
                const std::string notFoundHttpRes = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 51\r\n\r\n<html>404 Not Found</html>";
                send(clientSocket, notFoundHttpRes.c_str(), notFoundHttpRes.size(), 0);
                std::cout << "Server response: " << notFoundHttpRes << std::endl;
            }
        }

        // Close the client socket
        close(clientSocket);
        std::cout << "Client socket closed on server side." << std::endl << std::endl;
    }
    
    // close the server socket
    close(servSocket);
    
    serverClosed = true;

    //close thread
    t.join();

    std::cout << "Server closed." << std::endl << std::endl;

    exit(0);  // Explicitly terminate program
}

std::string readFile(const std::string &fileName)
{
    std::ifstream file(fileName);

    if (!file.is_open())
    {
        std::cerr << "File: " << fileName << " failed to open." << std::endl;
        return "";
    }

    std::string fileContent;

    std::ostringstream fileContentStream;
    fileContentStream << file.rdbuf();

    // close the file
    file.close();

    return fileContentStream.str();
}

void exitServer()
{
    const std::string exitCommand = "stop";
    std::string cliInput = "";

    while(!serverClosed){
        if(!std::cin)
        {
            serverClosed = true;
            return;
        }
        
        std::getline(std::cin, cliInput);

        if(!cliInput.empty() && cliInput != "")
        {
            std::transform(cliInput.begin(), cliInput.end(), cliInput.begin(), ::tolower);

            if(cliInput == exitCommand)
            {
                serverClosed = true;
            }
        }
    }
}