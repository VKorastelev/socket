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
    
    struct sockaddr_un server; 

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

    memset(&server, 0, sizeof(struct sockaddr_un));

    server.sun_family = AF_LOCAL;

    strncpy(server.sun_path, MY_SOCK_PATH, sizeof(server.sun_path) - 1);

    if (-1 == connect(fd_soc, (struct sockaddr *) &server, sizeof(struct sockaddr_un)))
    {
        perror("Error in connect(...)");
        exit(EXIT_FAILURE);
    }

    snprintf(send_buf, sizeof(send_buf) - 1, "Hello!!! Client PID = %d", getpid());
    size_send_data = strlen(send_buf) + 1;

    printf("Send data string (size = %ld, len = %ld): %s\n",
            size_send_data,
            strlen(send_buf),
            send_buf);

    num_send_data = send(fd_soc, send_buf, size_send_data, 0);
    if (0 != errno || num_send_data != size_send_data)
    {
        perror("Error in send(...)");
        fprintf(stderr, "Partial send?\n");
        exit(EXIT_FAILURE);
    }

    printf("This client send %ld bytes\n\n", num_send_data);

    num_read_data = recv(fd_soc, rec_buf, sizeof(rec_buf) - 1, 0);
    if (-1 == num_read_data)
    {
        perror("Error in recv(...)");
        exit(EXIT_FAILURE);
    }
    else if(0 == num_read_data)
    {
        printf("The server closed connection\n");
        exit(EXIT_FAILURE);
    }

    rec_buf[num_read_data]='\0';
        
    printf("From server receive %ld bytes: %s\n", num_read_data, rec_buf);

    close(fd_soc);

    return 0;
}
