#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#define BUF_SIZE 100

int main(int argc, char *argv[])
{
    int fd_soc = 0;

    struct sockaddr_in server;

    int num_port;
    struct in_addr addr;

    char buf[BUF_SIZE];
    ssize_t size_buf_data = 0;
    ssize_t num_send_data = 0;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s port ip_address_v4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (0 == inet_aton(argv[2], &addr))
    {
        fprintf(stderr, "Invalid IP_v4 address (must be xxx.xxx.xxx.xxx)\n");
        fprintf(stderr, "Usage: %s port ip_address_v4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    num_port = atoi(argv[1]);

    if (num_port < 1024 || num_port > 49151)
    {
        fprintf(stderr, "Invalid port number (must be in the range: 1024-49151)\n");
        fprintf(stderr, "Usage: %s port ip_address_v4\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(num_port);
    server.sin_addr.s_addr = addr.s_addr;

    printf("Server IP address in network byte order:%d\n", server.sin_addr.s_addr);
    printf("Server port number in network byte order:%d\n", server.sin_port);

    fd_soc = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == fd_soc)
    {
        perror("Error in socket(...)");
        exit(EXIT_FAILURE);
    }
    
    if (-1 == connect(fd_soc, (struct sockaddr *) &server, sizeof(struct sockaddr_in)))
    {
        perror("Error in connect(...)");
        exit(EXIT_FAILURE);
    }

    snprintf(buf, sizeof(buf) - 1, "Hello!!! Client PID = %d", getpid());
    size_buf_data = strlen(buf) + 1;

    printf("Send data string (size = %ld, len = %ld): %s\n",
            size_buf_data,
            strlen(buf),
            buf);

    num_send_data = send(fd_soc, buf, size_buf_data, 0);
    if (0 != errno || num_send_data != size_buf_data)
    {
        perror("Error in send(...)");
        fprintf(stderr, "Partial send?\n");
        exit(EXIT_FAILURE);
    }

    printf("This client send %ld bytes\n\n", num_send_data);

    size_buf_data = recv(fd_soc, buf, sizeof(buf) - 1, 0);
    if (-1 == size_buf_data)
    {
        perror("Error in recv(...)");
        exit(EXIT_FAILURE);
    }

    buf[size_buf_data]='\0';
        
    printf("From server receive %ld bytes: %s\n", size_buf_data, buf);

    close(fd_soc);

    return 0;
}
