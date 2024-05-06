#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>

using namespace std;

class Server {
private:
    int serverSocket;
    vector<int> clientSockets;
    struct sockaddr_in serverAddr;
    int port;
    mutex clientSocketsMutex;

public:
    Server(int port);
    void startListening();
    void handleClient();
};


Server::Server(int port) : port(port) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Error creating server socket\n";
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    memset(&serverAddr, 0, sizeof(serverAddr));

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))<=1) {
        cerr << "Error binding server socket\n";
        exit(EXIT_FAILURE);
    }
}

void Server::startListening() {
    
    clientSocketsMutex.lock();
    clientSockets.clear();
    clientSocketsMutex.unlock();
    listen(serverSocket, 5);
    cout << "Server listening on port " << port << endl;

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

        if (clientSocket == -1) {
            cerr << "Error accepting client connection\n";
            continue;
        }

        cout << "New client connected: " << inet_ntoa(clientAddr.sin_addr) << endl;
        clientSockets.push_back(clientSocket);

        thread clientThread([this, clientSocket]() {
            char buffer[1024];
            while (true) {
                int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesReceived <= 0) {
                    cerr << "Error receiving message from client\n";
                    break;
                }

                buffer[bytesReceived] = '\0';
                if (bytesReceived > 0 && bytesReceived <= sizeof(buffer)) {
                        cout << "Received from client: " << buffer << endl;
                    }

                for (int socket : clientSockets) {
                    if (socket != clientSocket) {
                        send(socket, buffer, bytesReceived - 1, 0);
                    }
                    close(socket);
                }
            }
            handleClient();
            close(clientSocket);
        });

        clientThread.detach();
    }
}

int main() {
    int port = 8080;
    Server server(port);
    server.startListening();
    return 0;
}

