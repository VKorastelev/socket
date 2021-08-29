#include <stdio.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MY_SOCK_PATH "/tmp/mystream"

#define BUF_SIZE 100

int main(int argc, char *argv[])
{
    int ret = 0;

    int fd_soc = -1;
    
    struct sockaddr_un server; 

    char buf[BUF_SIZE];
    ssize_t size_buf_data = 0;
    ssize_t num_send_data = 0;

    fd_soc = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (-1 == fd_soc)
    {
        perror("Error in socket(...)");
        ret = EXIT_FAILURE;
        goto finally;
    }

    memset(&server, 0, sizeof(struct sockaddr_un));

    server.sun_family = AF_LOCAL;

    strncpy(server.sun_path, MY_SOCK_PATH, sizeof(server.sun_path) - 1);

    if (-1 == connect(fd_soc, (struct sockaddr *)&server, sizeof(struct sockaddr_un)))
    {
        perror("Error in connect(...)");
        ret = EXIT_FAILURE;
        goto finally;
    }

    snprintf(buf, sizeof(buf) - 1, "Hello!!! Client PID = %d", getpid());
    size_buf_data = strlen(buf) + 1;

    printf("\nSend data string (size = %ld, len = %ld): %s\n",
            size_buf_data,
            strlen(buf),
            buf);

    errno = 0;
    num_send_data = send(fd_soc, buf, size_buf_data, 0);
    if (0 != errno || num_send_data != size_buf_data)
    {
        perror("Error in send(...)");
        fprintf(stderr, "Partial send?\n");
        ret = EXIT_FAILURE;
        goto finally;
    }

    printf("This client send %ld bytes\n", num_send_data);

    size_buf_data = recv(fd_soc, buf, sizeof(buf) - 1, 0);
    if (-1 == size_buf_data)
    {
        perror("Error in recv(...)");
        ret = EXIT_FAILURE;
        goto finally;
    }
    else if(0 == size_buf_data)
    {
        printf("The server closed connection\n");
        ret = EXIT_FAILURE;
        goto finally;
    }

    buf[size_buf_data]='\0';
        
    printf("\nFrom server receive %ld bytes: %s\n", size_buf_data, buf);

 finally:

    if (-1 != fd_soc)
    {
        puts("close(fd_soc)");
        if (-1 == close(fd_soc))
        {
            perror("Error in close(fd_soc)");
            ret = EXIT_FAILURE;
        }
    }

    return ret;
}
