#define _BSD_SOURCE
#include "telemetry_sv.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int32_t createServer(const char* port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, optval, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE; // use wildcard IP address

    s = getaddrinfo(NULL, port, &hints, &result);

    if (s != 0)
        return -1;

    optval = 1;

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue; // error, try next address 

        if (bind (sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sfd);
    }

    return (rp == NULL) ? -1 : sfd;
}
