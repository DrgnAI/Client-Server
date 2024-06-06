#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void receiveMessages(int sock) {
    char buffer[1024] = {0};
    while (true) {
        int bytes_received = recv(sock, buffer, 1024, 0);
        if (bytes_received <= 0) break;
        std::cout << "Server: " << buffer << std::endl;
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    const char* server_ip = "127.0.0.1";
    const int PORT = 8080;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection Failed" << std::endl;
        return -1;
    }

    std::thread receiverThread(receiveMessages, sock);
    receiverThread.detach();

    std::string message;
    while (true) {
        std::getline(std::cin, message);
        send(sock, message.c_str(), message.length(), 0);
    }

    close(sock);
    return 0;
}
