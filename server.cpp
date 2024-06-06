#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

std::vector<int> clients;
std::mutex clients_mutex;

void broadcastMessage(const std::string& message) {
    clients_mutex.lock();
    for (int client : clients) {
        send(client, message.c_str(), message.length(), 0);
    }
    clients_mutex.unlock();
}

void handleClient(int client_socket) {
    char buffer[1024];
    while (true) {
        int bytes_received = recv(client_socket, buffer, 1024, 0);
        if (bytes_received <= 0) {
            clients_mutex.lock();
            clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
            clients_mutex.unlock();
            close(client_socket);
            break;
        }
        buffer[bytes_received] = '\0';
        broadcastMessage(buffer);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    const int PORT = 8080;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        clients_mutex.lock();
        clients.push_back(new_socket);
        clients_mutex.unlock();
        std::thread(handleClient, new_socket).detach();
    }

    return 0;
}
