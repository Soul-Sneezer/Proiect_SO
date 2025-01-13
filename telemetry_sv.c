#define _DEFAULT_SOURCE
#include "telemetry_sv.h"
#include "rw_func.h"
#include "dynamic_list.h"
#include "channel.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int32_t createServer(const char* port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    socklen_t addrlen;
    struct sockaddr_storage claddr;
    int cfd, sfd, optval, s;
    uint16_t message_len;

    char* channel_path;
    char* message;
    uint8_t opcode;
    uint8_t channel_len;
    
    tokid_t chn;

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
        errExit("Could not bind socket to any address.");

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
    
        while (1) {
            // process request
            // find out request type
            if (readn(cfd, &opcode, 1) <= 0) {
                break;
            }
            printf("received request type: %u\n", opcode);
            fflush(stdout);
            
            switch (opcode) {
                case REGISTER_CHANNEL:
                    if (readn(cfd, &channel_len, 1) < 0) {
                        errMsg("Failed to read size of channel path.");
                        break;
                    }

                    channel_path = (char*)malloc((channel_len + 1) * sizeof(char));
                    channel_path[channel_len] = '\0';

                    if (readn(cfd, channel_path, channel_len) < 0) {
                        errMsg("Failed to read channel path.");
                        break;
                    }

                    chn = channel_open(CHANNEL_BOTH, channel_path);

                    free(channel_path);
                    if (writen(cfd, &chn, 4) < 0) {
                        errMsg("Failed to send channel token to client.");
                    }
                    break;
                case CLOSE_CHANNEL:
                    if (readn(cfd, &chn, 4) < 0) {
                        errMsg("Failed to read channel token.");
                        break;
                    }

                    if (channel_close(chn) != 0) {
                        errMsg("Could not close channel.");
                    }

                    break;
                case READ_OPERATION:
                    if (readn(cfd, &chn, 4) < 0) {
                        errMsg("Failed to read size of channel path.");
                        break;
                    }
                    printf("channel token is %d\n", chn);

                    if (readn(cfd, &message_len, 2) < 0) {
                        errMsg("Failed to read size of message.");
                        break;
                    }

                    message = (char*)malloc((message_len + 1) * sizeof(char));
                    if (readn(cfd, message, message_len) < 0) {
                        errMsg("Failed to read message.");
                        free(message);
                        break;
                    }

                    printf("received message: %s\n", message);
                   
                    if (channel_post(chn, message) != 0) {
                        errMsg("Failed to post message to channel.");
                    }

                    free(message);
                    continue;
                case WRITE_OPERATION:
                    if (readn(cfd, &chn, 4) < 0) {
                        errMsg("Failed to read size of channel path.");
                        break;
                    }

                    message = channel_read(chn, NULL);
                    message_len = strlen(message);

                    printf("Writing to client!\n");
                    if (writen(cfd, &message_len, 2) < 0) {
                        errMsg("Failed to send message length to client. Will abort.");
                        break;
                    }

                    if (writen(cfd, message, message_len) < 0) {
                        errMsg("Failed to send message to client.");
                        break;
                    }
                    free(message);
                    break; 
                    /*
                case SUBSCRIBE_OPERATION:
                    addElemToList(new_tlm_sv.client_list, cfd);
                case NOTIFY_OPERATION:
                    if (readn(cfd, &parameters[1], 1) < 0) {
                        errMsg("Failed to read size of channel path.");
                        break;
                    }

                    printf("channel path length is %d\n", parameters[1]);

                    channel_path = (char*)malloc((parameters[1] + 1) * sizeof(char));
                    channel_path[parameters[1]] = '\0';

                    if (readn(cfd, channel_path, parameters[1]) < 0) {
                        errMsg("Failed to read channel path.");
                        free(channel_path);
                        break;
                    }
                    printf("channel path is %s\n", channel_path);
                */
                default:
                    errMsg("Unknown operation type.");
                    continue;
            }
        }
        if (close(cfd) == -1)
            errMsg("close");
    }

    free(channel_path);
}

int closeServer(tlm_sv_t sv)
{
   if (close(sv.sfd) == -1) {
       errMsg("close");
       return -1;
   }

   return 0;
}

int main()
{
    createServer(DEFAULT_PORT);
}
