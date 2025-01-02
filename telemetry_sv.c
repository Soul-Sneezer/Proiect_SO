#define _DEFAULT_SOURCE
#include "telemetry_sv.h"
#include "rw_func.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

size_t getMessage(const char* path, char* message) // barebones for now
{
    size_t n = strlen("Hello world!");
    message = (char*)malloc(n * sizeof(char));
    message = "Hello world!";

    return n;
}

int32_t createServer(const char* port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    socklen_t addrlen;
    struct sockaddr_storage claddr;
    int cfd, sfd, optval, s;

    char* channel_path;
    char* message;
    uint8_t parameters[4];

    tlm_sv_t new_tlm_sv;
    //new_tlm_sv.client_list = NULL;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV; // use wildcard IP address

    if((s = getaddrinfo(NULL, port, &hints, &result)) != 0)
            errExit("getaddrinfo");

    optval = 1;

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue; // error, try next address 

        new_tlm_sv.sfd = sfd;

        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
            errExit("setsockopt");

        if (bind (sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sfd);
    }

    if (rp == NULL)
        errMsg("Could not bind socket to any address.");

    if (listen(sfd, BACKLOG) == -1)
        errExit("listen");

    freeaddrinfo(result);

    for (;;) { // client handling loop
        // accept connection 
        addrlen = sizeof(struct sockaddr_storage);
        cfd = accept(sfd, (struct sockaddr*)&claddr, &addrlen);
        
        if(cfd == -1) {
            errMsg("accept");
            continue;
        }
    
        // process request
        // find out request type 
        if (readn(cfd, &parameters[0], 1) < 0) {
            close(cfd);
            errMsg("Failed to read request type.");
            continue;
        }
        printf("received request type: %u\n", parameters[0]);
        
        if (readn(cfd, &parameters[1], 1) < 0) {
            close(cfd);
            errMsg("Failed to read size of channel path.");
            continue;
        }

        channel_path = (char*)malloc((parameters[1] + 1) * sizeof(char));
        if (readn(cfd, channel_path, parameters[1]) < 0) {
            close(cfd);
            errMsg("Failed to read channel path.");
            free(channel_path);
            continue;
        }
        
        switch (parameters[0]) {
            case READ_OPERATION:
                if (readn(cfd, &parameters[2], 2) < 0) {
                    close(cfd);
                    errMsg("Failed to read size of message.");
                    continue;
                }

                printf("channel length is %d\n", parameters[2]);

                message = (char*)malloc((parameters[2] + 1) * sizeof(char));
                if (readn(cfd, message, parameters[2]) < 0) {
                    close(cfd);
                    errMsg("Failed to read message.");
                    free(message);
                    continue;
                }

                printf("received message: %s\n", message);

                if (writen(cfd, message, parameters[0]) < 0) {
                    close(cfd);
                    errMsg("Failed to send acknowledgment.");
                    continue;
                }
                break;
            case WRITE_OPERATION:
                int32_t message_len = getMessage(channel_path, message);

                if (writen(cfd, &message_len, 2) < 0) {
                    close(cfd);
                    errMsg("Failed to send message length to client. Will abort.");
                    continue;
                }

                if (writen(cfd, message, message_len) < 0) {
                    close(cfd);
                    errMsg("Failed to send message to client.");
                    continue;
                }

                break;
            default:
                errMsg("Unknown operation type.");
        }

        
        if (close(cfd) == -1)
            errMsg("close");
    }

    free(channel_path);
    free(message);
}

int closeServer(tlm_sv_t sv)
{
   close(sv.sfd);
}

int main()
{
    createServer(DEFAULT_PORT);
}