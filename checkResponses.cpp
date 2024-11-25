#include <iostream>
#include <cstring>      // For memset, memcpy
#include <cstdio>       // For perror
#include <sys/socket.h> // For socket functions
#include <arpa/inet.h>  // For inet_addr and sockaddr_in
#include <unistd.h>     // For close

int main() {
    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Define server address
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8378); // HTTP port

    // Convert IP address to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0) { // example.com
        perror("Invalid address / Address not supported");
        close(sock);
        return 1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    // Prepare HTTP GET request
    // const char* request = 
    //     "PUT test.html HTTP/1.1\r\n"
	// 	"Host: 127.0.0.1\r\n"
    //     "Content-Type: text/plain\r\n"
    //     "Content-Length: 92\r\n"
    //     "Connection: close\r\n\r\n"
    //     "This is some random text for testing purposes. It could be anything you'd like to test with!";
    const char* request =
        "POST /directory/youpi.bla HTTP/1.1\r\n"
        "Accept-Encoding: gzip\r\n"
        "Content-Type: test/file\r\n"
        "Host: localhost:8378\r\n"
        // "Transfer-Encoding: chunked\r\n"
        "Content-Length: 5\r\n"
        "User-Agent: Go-http-client/1.1\r\n\r\n"

        "yo yo";
        // "4\r\n"
        // "test\r\n"
        // "6\r\n"
        // "123456\r\n"
        // "0\r\n"
        // "\r\n";

    // Send the request
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Send failed");
        close(sock);
        return 1;
    }

    // Receive the response
    char buffer[4096];
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the response
        std::cout << buffer;
    }

    if (bytes_received < 0) {
        perror("Receive failed");
    }

    // Close the socket
    close(sock);
    return 0;
}
