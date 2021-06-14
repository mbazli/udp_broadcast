/**
 * @brief   This program response to broadcasted message from host
 * 
 * @author  Murtadha Bazli Tukimat (murtadhabazlitukimat@gmail.com)
 * @date    14th June 2021
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netdb.h>

#define BROADCAST_UDP_PORT  1112

int error_handler(int sock, char* source)
{
    perror(source);
    close(sock);
    return 1;
}

int main(void)
{
    int socked = socket(AF_INET, SOCK_DGRAM, 0);
    if(socked < 0)
    {
        return error_handler(socked, "socket");
    }

    // add socket option for broadcast
    int rc = 1;
    if(setsockopt(socked, SOL_SOCKET, SO_BROADCAST, &rc, sizeof(rc)) < 0)
    {
        return error_handler(socked, "setsockopt");
    }

    // bind to port 1112
    struct sockaddr_in recv_addr;
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(BROADCAST_UDP_PORT);
    recv_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(socked, (struct sockaddr*) &recv_addr, sizeof(recv_addr)) < 0)
    {
        return error_handler(socked, "bind");
    }

    // receive broadcast message from client
    struct sockaddr_in send_addr;
    char recv_buffer[25] = {0};
    int len = sizeof(send_addr);
    if(recvfrom(socked, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*) &send_addr, &len) < 0)
    {
        return error_handler(socked, "recvfrom");
    }

    // send back dummy message
    char send_buffer = 1;
    if(sendto(socked, &send_buffer, sizeof(send_buffer), 0, (struct sockaddr*) &send_addr, sizeof(send_addr)) < 0)
    {
        return error_handler(socked, "sendto");
    }

    return 0;
}