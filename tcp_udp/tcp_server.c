#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>

/* socket
 * bind
 * listen
 * accept
 * send/recv
 */

#define SERVER_PORT 8888
#define BACKLOG 10
#define RECV_BUF_LEN 1024

int main(int argc, char** argv)
{
    signal(SIGCHLD, SIG_IGN);
    
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == server_socket)
    {
        perror("socket");
        return errno;
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    int ret = bind(server_socket, (const struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if (-1 == ret)
    {
        perror("bind");
        return errno;
    }

    ret = listen(server_socket, BACKLOG);
    if (-1 == ret)
    {
        perror("listen");
        return errno;
    }

    int32_t client_num = 0;
    socklen_t addr_len = 0;
    int client_socket;
    struct sockaddr_in client_addr;
    while(1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        client_num++;
        if (-1 == client_socket)
        {
            printf("fail to accept, errno: %d", errno);
            continue;
        }

        if(fork() == 0)
        {
            while(1)
            {
                uint8_t recv_buf[RECV_BUF_LEN];
                int recv_len = recv(client_socket, recv_buf, RECV_BUF_LEN - 1, 0);
                if (recv_len <= 0)
                {
                    close(client_socket);
                    return errno;
                }
                
                recv_buf[recv_len] = '\0';
                printf("client %d: %s", client_num, recv_buf);
            }
        }
    }
}