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
std::string generateRes(int code, const std::string fileContent, const std::string messageBody);

int main()
{
    const int portNum = 60001;
    char buffer[BUFFER_SIZE] = {0};
    std::string loopPrintLinesH1 = "================================================================";
    std::string loopPrintLinesH2 = "----------------------------------------------------------------";

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
 
    //Open thread
    std::thread t(exitServer);

    // accept client connections
    while (!serverClosed)
    {
        std::cout << loopPrintLinesH1 << std::endl;
        std::cout << "Server listening on port " << portNum << "..." << std::endl << std::endl;
        std::cout << loopPrintLinesH2 << std::endl;

        int clientSocket = accept(servSocket, nullptr, nullptr); // server socket to interact with client, structure like before - if you know - else nullptr for local connection

        if(serverClosed)
        {
            std::cout << "Server closed. Closing client socket now..." << std::endl;
            close(clientSocket);
            break;
        }

        if (clientSocket < 0)
        {
            perror("Error: Failed to accept client.");
            continue;
        }

        // Receive the HTTP request
        int bytesRecvd = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRecvd == 0)
        {
            std::cout << "Client disconnected.\n";
            close(clientSocket);
            continue;
        }
        else if (bytesRecvd < 0)
        {
            std::cout << "Client request: no request\nRead from client failed" << std::endl;
            perror("Client request: no request\nRead from client failed");
            close(clientSocket);
            continue;
        }

        buffer[bytesRecvd] = '\0';

        // get the file name from the http request
        std::istringstream httpReqStream(buffer);
        std::string method, path, line;
        httpReqStream >> method >> path;

        std::cout << "Client Request: " << std::endl;
        std::cout << "\tmethod: " << method << std::endl;
        std::cout << "\tpath: " << path << std::endl << std::endl;

        int httpCodeOK = 200;
        int httpCodeNoContent = 204;
        int httpCodeNotFound = 404;

        // check method type
        if (method == "POST")
        {
            // retrieve message from client
            std::string headers;
            while (std::getline(httpReqStream, line) && line != "\r")
            {
                headers += line + "\n";
            }

            // Read the request body (message payload) from the client
            std::string message;
            std::getline(httpReqStream, message);

            // replace + with spaces
            std::replace(message.begin(), message.end(), '+', ' ');
            // remvove "message="
            message.erase(0, 8);
            std::cout << "Client Request Body: " << message << std::endl << std::endl;

            if(message == "stop")
            {
                serverClosed = true;
                std::cout << "Received 'stop' command from client. Server shutting down." << std::endl;
                break;
            }

            //redirect them back to the main screen
            path = "index.html";
            //std::string fileContent = readFile(path);
            std::string serverRes = generateRes(httpCodeNoContent, "", message); // Generates the initial response without the file content.

            send(clientSocket, serverRes.c_str(), serverRes.size(), 0);
            std::cout << "Server response: " << std::endl;
            std::cout << serverRes << std::endl;
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
                std::string serverRes = generateRes(httpCodeOK, fileContent, ""); // Generates the initial response without the file content.
                std::string httpRes = serverRes + fileContent; // The server's response needs the file content added to it at this point. 
    
                send(clientSocket, httpRes.c_str(), httpRes.size(), 0);
                std::cout << "Server response: " << std::endl;
                std::cout << serverRes << std::endl;
            }
            else // file path choosen is not valid, send "404 not found"
            {
                std::string serverRes = generateRes(httpCodeNotFound, "", "");
                send(clientSocket, serverRes.c_str(), serverRes.size(), 0);
                std::cout << "Server response: \n" << serverRes << std::endl;
            }
        }

        std::cout << "Server closed client socket: " << clientSocket << std::endl << std::endl;

        // Close the client socket
        close(clientSocket);
        std::cout << loopPrintLinesH2 << std::endl;
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

std::string generateRes(int code, const std::string fileContent, const std::string messageBody)
{
    if(code == 200)
    {
        std::string resHeader = "HTTP/1.1 200 OK\r\n";
        std::string resContentType = "Content-Type: text/html\r\n";
        std::string strFileSize = std::to_string(fileContent.size());
        std::string resContentLength = "Content-Length: " + strFileSize + "\r\n\r\n";

        return resHeader + resContentType + resContentLength;
    }else if(code == 204)
    {
        std::string resHeader = "HTTP/1.1 206 No Content\r\n";
        std::string resContentType = "Content-Type: text/html\r\n";
        std::string strMsgSize = std::to_string(messageBody.size());
        std::string resContentLength = "Content-Length: " + strMsgSize + "\r\n\r\n";

        return resHeader + resContentType + resContentLength;
    }else
    {
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 51\r\n\r\n<html>404 Not Found</html>";
    } 
}