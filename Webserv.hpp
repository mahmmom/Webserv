#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <map>
#include <string>
#include <vector>

class Client {
public:
    Client(int socket) : socket(socket) {}
    std::string& getBuffer() { return buffer; }
private:
    int socket;
    std::string buffer;
};

class Webserv {
public:
    Webserv(const std::string& config_file);
    ~Webserv();
    void run();

private:
    static const int MAX_EVENTS = 1024;

    void setNonBlocking(int socket);
    void acceptNewClient(int server_socket);
    void handleClientRead(int client_socket);
    void handleClientWrite(int client_socket);
    void removeClient(int client_socket);
    void parseConfigFile(const std::string& config_file);

    std::vector<int> server_sockets;
    int kq;
    std::map<int, Client> clients;
};

#endif // WEBSERV_HPP