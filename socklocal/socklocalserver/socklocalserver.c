#include <stdio.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MY_SOCK_PATH "/tmp/mystream"
#define LISTEN_QUEUE_LEN 5

#define BUF_SIZE 100


int main(int argc, char *argv[])
{
    int fd_soc = 0;

    int fd_new_soc = 0;

    struct sockaddr_un server;
    struct sockaddr_un client;
    socklen_t client_addr_size;

    char send_buf[BUF_SIZE];
    char rec_buf[BUF_SIZE];
    ssize_t size_send_data = 0;
    ssize_t num_send_data = 0;
    ssize_t num_read_data = 0;

    fd_soc = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (-1 == fd_soc)
    {
        perror("Error in socket(...)");
        exit(EXIT_FAILURE);
    }

    if (-1 == remove(MY_SOCK_PATH) && ENOENT != errno)
    {
        fprintf(stderr, "Error in remove (%s)\n", MY_SOCK_PATH);
        exit(EXIT_FAILURE);
    }


    memset(&server, 0, sizeof(struct sockaddr_un));

    server.sun_family = AF_LOCAL;
    
    strncpy(server.sun_path, MY_SOCK_PATH, sizeof(server.sun_path) - 1);

    if (-1 == bind(fd_soc, (struct sockaddr *) &server, sizeof(struct sockaddr_un)))
    {
        perror("Error in bind(...)");
        exit(EXIT_FAILURE);
    }

    if (-1 == listen(fd_soc, LISTEN_QUEUE_LEN))
    {
        perror("Error in listen(...)");
        exit(EXIT_FAILURE);
    }

    printf("Server listen: %s\n", MY_SOCK_PATH);

    client_addr_size = sizeof(struct sockaddr_un);
    
    fd_new_soc = accept(fd_soc, (struct sockaddr *) &client, &client_addr_size);
    if (-1 == fd_new_soc)
    {
        perror("Error in accept(...)");
        exit(EXIT_FAILURE);
    }

    num_read_data = recv(fd_new_soc, rec_buf, sizeof(rec_buf) - 1, 0);
    if (-1 == num_read_data)
    {
        perror("Error in recv(...)");
        exit(EXIT_FAILURE);
    }
    else if(0 == num_read_data)
    {
        printf("The client closed connection\n");
        exit(EXIT_FAILURE);
    }

    rec_buf[num_read_data]='\0';
        
    printf("From client receive %ld bytes: %s\n\n", num_read_data, rec_buf);

    snprintf(send_buf, sizeof(send_buf) - 1, "This server! Server PID = %d",
            getpid());
        
    size_send_data = strlen(send_buf) + 1;

    printf("Send data string (size = %ld, len = %ld): %s\n",
            size_send_data,
            strlen(send_buf),
            send_buf);

    num_send_data = send(fd_new_soc, send_buf, size_send_data, 0);
    if (0 != errno || num_send_data != size_send_data)
    {
        perror("Error in send(...)");
        fprintf(stderr, "Partial send?\n");
        exit(EXIT_FAILURE);
    }

    printf("This server send %ld bytes\n", num_send_data);
    
    close(fd_new_soc);

    close(fd_soc);

    if (0 != unlink(MY_SOCK_PATH))
    {
        perror("Error in unlink()");
        exit(EXIT_FAILURE);
    }
    return 0;
}
