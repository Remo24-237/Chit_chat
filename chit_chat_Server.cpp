#include <iostream>  // for input-output operations
#include <cstdlib>   // for exit() function
#include <cstring>   // for memset() function
#include <unistd.h>  // for socket programming
#include <sys/socket.h>  // for socket programming
#include <netinet/in.h>  // for sockaddr_in structure
#include <arpa/inet.h>  // for inet_ntoa() function
#include <thread>  // for multithreading
#include <vector>  // for storing client sockets
#include <mutex>  // for thread-safe access to clientSockets vector
#include <thread>

using namespace std;

// A Server class that handles multiple clients
class Server {
private:
    int serverSocket;  // the server socket
    vector<int> clientSockets;  // vector to store connected client sockets
    struct sockaddr_in serverAddr;  // server address structure
    int port;  // the port number on which the server listens
    mutex clientSocketsMutex;  // mutex for thread-safe access to clientSockets

public:
    // Constructor to initialize the server with a port number
    Server(int port) : port(port) {
        // Create a server socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            cerr << "Error creating server socket\n";
            exit(EXIT_FAILURE);
        }

        // Initialize server address structure
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;  // listen on all available network interfaces
        serverAddr.sin_port = htons(port);  // convert port to network byte order

        // Initialize server address structure to zero
        memset(&serverAddr, 0, sizeof(serverAddr));

        // Bind the server socket to the specified port
        if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
            cerr << "Error binding server socket\n";
            exit(EXIT_FAILURE);
        }
    }

    // Start listening for incoming client connections
    void startListening() {
        // Clear the clientSockets vector and lock it for thread-safe access
        clientSocketsMutex.lock();
        clientSockets.clear();
        clientSocketsMutex.unlock();

        // Listen for incoming connections
        listen(serverSocket, 10);
        cout << "Server listening on port " << port << endl;

        while (true) {
            // Accept incoming client connection
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

            if (clientSocket == -1) {
                cerr << "Error accepting client connection\n";
                continue;
            }

            // Print connection status
            cout << "New client connected: " << inet_ntoa(clientAddr.sin_addr) << endl;

            // Add the client socket to the vector
            clientSocketsMutex.lock();
            clientSockets.push_back(clientSocket);
            clientSocketsMutex.unlock();

            // Create a new thread to handle the client
            thread clientThread([this, clientSocket](int clientSocket) {
                char buffer[1024];  // buffer to receive client messages
                while (true) {
                    // Receive message from client
                    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                    if (bytesReceived <= 0) {
                        cerr << "Error receiving message from client\n";
                        break;
                    }

                    // Null-terminate the received message
                    buffer[bytesReceived] = '\0';

                    // Print the received message
                    if (bytesReceived > 0 && bytesReceived <= sizeof(buffer)) {
                        cout << "Received from client: " << buffer << endl;
                    }

                    // Broadcast the message to all connected clients
                    for (int socket : clientSockets) {
                        if (socket!= clientSocket) {
                            send(socket, buffer, bytesReceived - 1, 0);
                        }
                        close(socket);  // close the socket after sending the message
                    }
                }

                // Handle client disconnection
                handleClient();

                // Close the client socket
                close(clientSocket);
            },clientSocket);

            // Detach the client thread to allow it to run independently
            clientThread.detach();
        }
    }

    // Handle client disconnection (not implemented in this code)
    void handleClient() {
        // TODO: implement client disconnection logic
    }
};

int main() {
    int port = 8080;  // the port number to listen on
    Server server(port);  // create a server instance
    server.startListening();  // start listening for incoming connections
    return 0;
}