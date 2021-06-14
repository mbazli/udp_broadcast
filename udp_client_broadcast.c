/**
 * @brief   This program send broadcast message to all device within same subnet.
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
#include <ifaddrs.h>

#define BROADCAST_UDP_PORT      1112
#define DEVICE_RES_TIMEOUT_SEC  3

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

    // add socket option for receive timeout
    struct timeval read_timeout;
    read_timeout.tv_sec = DEVICE_RES_TIMEOUT_SEC;
    read_timeout.tv_usec = 0;
    if(setsockopt(socked, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout)) < 0)
    {
        return error_handler(socked, "setsockopt");
    }

    // get all network interface information
    struct ifaddrs *ifaddr;
    if(getifaddrs(&ifaddr) < 0)
    {
        return error_handler(socked, "getifaddrs");
    }

    // iterate linked list of interface information
    int family;
    for(struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        family = ifa->ifa_addr->sa_family;

        // check for ipv4 family
        if(family == AF_INET)
        {
            char buff[64] = {0};
            printf("iface name: %s\n", ifa->ifa_name);

            // get IP address
            inet_ntop(family, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, buff, sizeof(buff));
            printf("IP addr: %s\n", buff);

            // get netmask
            inet_ntop(family, &((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr, buff, sizeof(buff));
            printf("netmask: %s\n", buff);

            // get broadcast address
            inet_ntop(family, &((struct sockaddr_in *)ifa->ifa_ifu.ifu_broadaddr)->sin_addr, buff, sizeof(buff));
            printf("broadcast addr: %s\n", buff);

            // send broadcast message
            struct sockaddr_in broadcast_addr;
            broadcast_addr.sin_family = family;
            broadcast_addr.sin_addr.s_addr = inet_addr(buff);
            broadcast_addr.sin_port = htons(BROADCAST_UDP_PORT);

            char send_buffer[25] = "Broadcast message..";
            if(sendto(socked, send_buffer, strlen(send_buffer), 0, (struct sockaddr *) &broadcast_addr, sizeof(broadcast_addr)) < 0)
            {
                return error_handler(socked, "sendto");
            }
            printf("\n");
        }
    }

    // another loop to receive message response from device
    for(struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        family = ifa->ifa_addr->sa_family;

        // check for ipv4 family
        if(family == AF_INET)
        {
            char buff[64] = {0};
            char recv_buffer;
            struct sockaddr_in dev_addr;
            int len = sizeof(dev_addr);
            if(recvfrom(socked, &recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *) &dev_addr, &len) > 0)
            {
                inet_ntop(family, &dev_addr.sin_addr, buff, sizeof(buff));
                printf("response from: %s\n", buff);
            }
        }
    }

    // free linked list
    freeifaddrs(ifaddr);

    return 0;
}