#include <stdio.h>      /* for fprintf() */
#include <winsock2.h>   /* for socket(), connect(), send(), and recv() */
#include <ws2tcpip.h>   /* for ip options */
#include <stdlib.h>     /* for atoi() and exit() */

#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#endif


static void DieWithError(char* errorMessage)
{
    fprintf(stderr, "%s\n", errorMessage);
    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
    SOCKET    sock;                   /* Socket */
    WSADATA   wsaData;                /* For WSAStartup */
    char*     multicastIP;            /* Arg: IP Multicast address */
    char*     multicastPort;          /* Arg: Server port */
    char*     sendString;             /* Arg: String to multicast */
    size_t    sendStringLen;          /* Length of string to multicast */
    DWORD     multicastTTL;           /* Arg: TTL of multicast packets */
    ADDRINFO* multicastAddr;          /* Multicast address */
    ADDRINFO  hints          = { 0 }; /* Hints for name lookup */

    if ( WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        DieWithError("WSAStartup() failed");
    }

    if ( argc < 4 || argc > 5 )
    {
        fprintf(stderr, "Usage:  %s <Multicast Address> <Port> <Send String> [<TTL>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    multicastIP   = argv[1];             /* First arg:   multicast IP address */
    multicastPort = argv[2];             /* Second arg:  multicast port */
    sendString    = argv[3];             /* Third arg:   String to multicast */
    multicastTTL  = (argc == 5 ?         /* Fourth arg:  If supplied, use command-line */
                     atoi(argv[4]) : 1); /* specified TTL, else use default TTL of 1 */
    sendStringLen = strlen(sendString);  /* Find length of sendString */

    /* Resolve destination address for multicast datagrams */
    hints.ai_family   = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_NUMERICHOST;
    if ( getaddrinfo(multicastIP, multicastPort, &hints, &multicastAddr) != 0 )
    {
        DieWithError("getaddrinfo() failed");
    }

    printf("Using %s\n", multicastAddr->ai_family == PF_INET6 ? "IPv6" : "IPv4");

    /* Create socket for sending multicast datagrams */
    if ( (sock = socket(multicastAddr->ai_family, multicastAddr->ai_socktype, 0)) == INVALID_SOCKET )
    {
        DieWithError("socket() failed");
    }

    /* Set TTL of multicast packet */
    if ( setsockopt(sock,
                    multicastAddr->ai_family == PF_INET6 ? IPPROTO_IPV6        : IPPROTO_IP,
                    multicastAddr->ai_family == PF_INET6 ? IPV6_MULTICAST_HOPS : IP_MULTICAST_TTL,
                    (char*) &multicastTTL, sizeof(multicastTTL)) != 0 )
    {
        DieWithError("setsockopt() failed");
    }

    for (;;) /* Run forever */
    {
        if ( sendto(sock, sendString, sendStringLen, 0,
                    multicastAddr->ai_addr, multicastAddr->ai_addrlen) != sendStringLen )
        {
            DieWithError("sendto() sent a different number of bytes than expected");
        }

        Sleep(3000); /* Multicast sendString in datagram to clients every 3 seconds */
    }

    /* NOT REACHED */
    freeaddrinfo(multicastAddr);
    closesocket(sock);
    return 0;
}