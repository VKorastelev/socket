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

int print_sockaddr_un(int const sockfd);

int main(int argc, char *argv[])
{
    int ret = 0;

    int fd_soc = -1;
    int fd_new_soc = -1;

    struct sockaddr_un server;
    struct sockaddr_un client;
    socklen_t client_addr_size = 0;

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

    if (-1 == remove(MY_SOCK_PATH) && ENOENT != errno)
    {
        fprintf(stderr, "Error in remove (%s)\n", MY_SOCK_PATH);
        ret = EXIT_FAILURE;
        goto finally;
    }

    memset(&server, 0, sizeof(struct sockaddr_un));

    server.sun_family = AF_LOCAL;
    
    strncpy(server.sun_path, MY_SOCK_PATH, sizeof(server.sun_path) - 1);

    if (-1 == bind(fd_soc, (struct sockaddr *)&server, sizeof(struct sockaddr_un)))
    {
        perror("Error in bind(...)");
        ret = EXIT_FAILURE;
        goto finally;
    }
    
    if (-1 == listen(fd_soc, LISTEN_QUEUE_LEN))
    {
        perror("Error in listen(...)");
        ret = EXIT_FAILURE;
        goto finally;
    }

    printf("Server listen: %s\n", MY_SOCK_PATH);
    
    client_addr_size = sizeof(struct sockaddr_un);
    memset(&client, 0, client_addr_size);
    printf("Before accept(...) client_addr_size = %d\n", client_addr_size);

    fd_new_soc = accept(fd_soc, (struct sockaddr *)&client, &client_addr_size);
    if (-1 == fd_new_soc)
    {
        perror("Error in accept(...)");
        ret = EXIT_FAILURE;
        goto finally;
    }

    printf("After accept(...) client_addr_size = %d\n", client_addr_size);

    if (-1 == print_sockaddr_un(fd_new_soc))
    {
        puts("Error in print_sockaddr_un(...)");
        ret = EXIT_FAILURE;
        goto finally;
    }

    size_buf_data = recv(fd_new_soc, buf, sizeof(buf) - 1, 0);
    if (-1 == size_buf_data)
    {
        perror("Error in recv(...)");
        ret = EXIT_FAILURE;
        goto finally;
    }
    else if(0 == size_buf_data)
    {
        printf("The client closed connection\n");
        ret = EXIT_FAILURE;
        goto finally;
    }

    buf[size_buf_data]='\0';
        
    printf("\nFrom client receive %ld bytes: %s\n", size_buf_data, buf);

    snprintf(buf, sizeof(buf) - 1, "This server! Server PID = %d",
            getpid());
        
    size_buf_data = strlen(buf) + 1;

    printf("\nSend data string (size = %ld, len = %ld): %s\n",
            size_buf_data,
            strlen(buf),
            buf);

    errno = 0;
    num_send_data = send(fd_new_soc, buf, size_buf_data, 0);
    if (0 != errno || num_send_data != size_buf_data)
    {
        perror("Error in send(...)");
        fprintf(stderr, "Partial send?\n");
        ret = EXIT_FAILURE;
        goto finally;
    }

    printf("This server send %ld bytes\n", num_send_data);

 finally:
    
    if (-1 != fd_new_soc)
    {
        puts("close(fd_new_soc)");
        if (-1 == close(fd_new_soc))
        {
            perror("Error in close(fd_new_soc)");
            ret = EXIT_FAILURE;
        }
    }

    if (-1 != fd_soc)
    {
        puts("close(fd_soc)");
        if (-1 == close(fd_soc))
        {
            perror("Error in close(fd_soc)");
            ret = EXIT_FAILURE;
        }
    }

    if (-1 == remove(MY_SOCK_PATH) && ENOENT != errno)
    {
        fprintf(stderr, "Error in remove (%s)\n", MY_SOCK_PATH);
        ret = EXIT_FAILURE;
    }

    return ret;
}

int print_sockaddr_un(int const sockfd)
{
    int ret = 0;

    void *paddr = NULL;
    socklen_t addrsize = sizeof(struct sockaddr_un);

    paddr = malloc(addrsize + 1);
    if (NULL == paddr)
    {
        perror("Error in malloc(...), function print_sockaddr_un(...)");
        ret = -1;
        goto finally;
    }
    memset(paddr, 0, addrsize + 1);

    printf("Before getsockname(...) addrsize = %d\n", addrsize);

    if (-1 == getsockname(sockfd, (struct sockaddr *)paddr, &addrsize))
    {
        perror("Error in getsockname(...)");
        ret = -1;
        goto finally;
    }

    printf("After getsockname(...) addrsize = %d\n", addrsize);
    printf("In struct sockaddr_un sun_family = %d, sun_path = %s\n",
            ((struct sockaddr_un *)paddr)->sun_family, 
            ((struct sockaddr_un *)paddr)->sun_path);

 finally:

    if (NULL != paddr)
    {
        free(paddr);
    }

    return ret;
}
