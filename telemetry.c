#define _BSD_SOURCE

#include <syslog.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "telemetry.h"

static bool isDecimal(char c)
{
    return c >= '0' && c <= '9';
}

static bool isAlphanumeric(char c)
{
    return isDecimal(c) || (c >= 'a'  && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool isValidIP(const char* ip) // checks if it's a valid IP(v4 or v6), domain names are also accepted
{
    if(ip == NULL || *ip == '\0') // ip can be null if server is on localhost
        return true;

    size_t n = strlen(ip);
    if (n > 253) // clearly not an IP, and not a domain name either
        return false;
    else if(n == 39) { //
        for (int i = 0; i < n; i++) {
            if (i % 4 == 0 && ip[i] != ':') {
                return false;
            }
            else if(!isAlphanumeric(ip[i])) {
                return false;
            }
        }
    }

    bool ipv4 = true;
    uint8_t dots = 0;
    uint32_t value = 0;
    
    for (int i = 0; i < n; i++) {
        if(!isDecimal(ip[i])) // not ipv4 must be a domain
        {
            ipv4 = false;
            break;
        
        } else if (ip[i] == '0' && value == 0) {
            ipv4 = false;
        } else {
            value = value * 10 + (ip[i] - '0');
        }

        if(ip[i] == '.') {
            if(value > 255) {
                ipv4 = false;
                break;
            }
            dots++;
            value = 0;
        }  
    }

    if(dots != 3) // means that the last part of the string was of the form  .number, so it's not a domain either
        return false; 
    else if(value > 255)
        return false;
    
    if(ipv4)
        return true;
    else { // it must be a domain name 
        uint8_t subdomain_length = 0;
        for (int i = 0; i < 255; i++)
        {
            if(!isAlphanumeric(ip[i]) && ip[i] != '.' && ip[i] != '-') // unknown character
                return false;
            else if(ip[i] == '.') {
                if(subdomain_length == 0) // subdomain length must be at least 1
                    return false;
                
                subdomain_length = 0;
            } else {
                subdomain_length++;
            }

            if(subdomain_length > 63) {
                return false;
            }
        }
    }

    return true;
}

static bool isValidChannelName(const char* channel_name)
{
    if(channel_name == NULL || *channel_name == '\0') // channel name can't be null
        return false;

    if(channel_name[0] != '/')
        return false;

    size_t n = strlen(channel_name);
    if(channel_name[n - 1] == '/')
        return false;

    while(*channel_name != '\0') {
        if(!isAlphanumeric(*channel_name) && *channel_name != '/' && *channel_name != '_')
            return false;
        channel_name++;
    }

    return true;
}

static int isValidPort(const char* port)
{
    if(port == NULL || *port == '\0') // port can be null
        return 0;

    uint32_t value = 0;

    while(*port != '\0') {
        if(!isDecimal(*port))
            return -1;
        value = value * 10 + (*port - '0');
        port++;
    }

    if(value < 1024 || value > 65565)
        return -1;

    return value;
}

tlm_t tlm_open(uint8_t type, const char* channel_name, const char* ip, const char* port)
{
    tlm_t new_tlm;
    uint32_t port_value;

    if (!isValidIP(channel_name)) {
        fatal("Invalid IP. Expected input is an IPv4 address of the form:'n.n.n.n' where n is a 8 bit unsigned value OR\n \ 
                                    an IPv6 address of the form:'hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh' where h is a hexadecimal value OR\n \
                                    a domain name."); 
    }

    if ((port_value = isValidPort(port)) == -1) {
        fatal("Invalid port. Port value can be anything between 1024 and 65565(both inclusive).");
    }

    if (!isValidChannelName(channel_name)) {
        fatal("Invalid channel name. Must be a valid absolute UNIX path.");
    }

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s; 

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    s = getaddrinfo(ip, port, &hints, &result);

    if (s != 0 ) {
        errno = ENOSYS;
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue; // error, try next address 

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; // successfully connected, exit loop

        close(sfd); // connection failed, close this socket and try next address
    }

    freeaddrinfo(result);

    return (rp == NULL) ? -1 : sfd;
}

int32_t tlm_callback(tlm_t token, void (*message_callback)(tlm_t token, const char* message))
{
}

const char* tlm_read(tlm_t token, uint32_t* message_id)
{
}

int tlm_post(tlm_t token, const char* message)
{
}

void tlm_close(tlm_t token)
{
    close(token);
}

int tlm_type(tlm_t token)
{
}
