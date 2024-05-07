#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

void startClient(const char* serverIp, int port) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Error creating client socket\n";
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Error connecting to server\n";
        exit(EXIT_FAILURE);
    }

    char message[1024];

    while (true) {
        cout << "Enter message: ";
        cin.getline(message, sizeof(message));

        send(clientSocket, message, strlen(message), 0);

        if (strcmp(message, "exit") == 0) {
            break;
        }
    }

    close(clientSocket);
}
int main() {
    const char* serverIp = "127.0.0.1"; // Replace with server IP
    int port = 8080; // Same port as the server
    startClient(serverIp, port);
    return 0;
}
