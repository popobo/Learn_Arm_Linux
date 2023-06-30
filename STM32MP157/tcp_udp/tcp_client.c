
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 8888
#define SEND_BUF_LEN 1024

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <server IP>", argv[0]);
        return -1;
    }

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (0 ==  inet_aton(argv[1], &server_addr.sin_addr))
    {
        printf("invalid server IP");
        return -1;
    }

    int ret = connect(client_socket, (const struct sockaddr*)&server_addr, sizeof(struct sockaddr));
    if (-1 == ret)
    {
        printf("fail to connect, errno: %d", errno);
        return -1;
    }

    uint8_t send_buf[SEND_BUF_LEN];
    while(1)
    {
        fgets(send_buf, SEND_BUF_LEN, stdin);
        size_t send_len = send(client_socket, send_buf, strlen(send_buf), 0);
        if (send_len <= 0)
        {
            close(client_socket);
            return -1;
        }
    }

    return 0;
}
