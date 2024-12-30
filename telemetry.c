#define _DEFAULT_SOURCE

#include <syslog.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "telemetry.h"
#include "rw_func.h"

static bool isDecimal(const char c)
{
    return c >= '0' && c <= '9';
}

static bool isAlphanumeric(const char c)
{
    return isDecimal(c) || (c >= 'a'  && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool isValidDomain(const char* domain)
{
    size_t len = strlen(domain);
    uint8_t subdomain = 0;
    bool dot = false;

    if (len > 253 || len < 4) { // mimimum domain name length is 4, I believe (example domain: a.co)
        return false;
    }

    for (size_t i = 0; i < len; i++) {
        if (domain[i] == '.') {
            if (subdomain == 0)
                return false;
            subdomain = 0;
            dot = true;
        } else {
            if (!isAlphanumeric(domain[i]) && domain[i] != '-')
                return false; 

            subdomain++;
            if (subdomain > 63)
                return false;
        }
    }

    if (!dot)
        return false;

    return true;
}

static bool isValidIP(const char* ip) // checks if it's a valid IP(v4 or v6), domain names are also accepted
{
    struct in_addr ipv4_addr;
    struct in6_addr ipv6_addr;

    return inet_pton(AF_INET, ip, &ipv4_addr) == 1 || inet_pton(AF_INET6, ip, &ipv6_addr) == 1 || isValidDomain(ip); 
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
    tlm_t new_tlm = {-1};

    new_tlm.type = type;
    uint32_t port_value;

    if (!isValidIP(ip)) {
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
    
    size_t n = strlen(channel_name);
    new_tlm.channel_path = (char*)malloc((n + 1) * sizeof(char));
    strncpy(new_tlm.channel_path, channel_name, n);
    new_tlm.channel_path[n] = '\0';
    
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
        return new_tlm;
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

    new_tlm.sfd = sfd;
    return new_tlm;
}

int32_t tlm_callback(tlm_t token, void (*message_callback)(tlm_t token, const char* message))
{

}

const char* tlm_read(tlm_t token, uint32_t* message_id)
{

}

int tlm_post(tlm_t token, const char* message)
{
    size_t message_length = strlen(message);
    size_t channel_length = strlen(token.channel_path);
    
    if (message_length > 255) {
    }

    if (channel_length > 255) {
    }

    uint8_t msg_len = message_length;
    uint8_t chn_len = channel_length;
    uint8_t opcode = 3;

    // send information to server
    if (writen(token.sfd, &opcode, 1) < 0) {
        return -1;
    }
    
    if (writen(token.sfd, &chn_len, 1) < 0) {
        return -1;
    }

    if (writen(token.sfd, &msg_len, 1) < 0) {
        return -1;
    }

    if (writen(token.sfd, token.channel_path, channel_length) < 0) {
        return -1;
    }

    if (writen(token.sfd, message, message_length) < 0) {
        return -1;
    }

    return 0;
}

void tlm_close(tlm_t token)
{
    free(token.channel_path);
    close(token.sfd);
}

int tlm_type(tlm_t token)
{
    return token.type;
}

int main()
{
    tlm_t ntlm = tlm_open(1, "/home/user/channel/a", "127.0.0.1", DAEMON_DEFAULT_PORT);
    tlm_post(ntlm, "Hello World");
}
