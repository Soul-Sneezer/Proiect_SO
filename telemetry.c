#define _DEFAULT_SOURCE

#include "telemetry.h"
#include "rw_func.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <syslog.h>

static bool isDecimal(const char c) { return c >= '0' && c <= '9'; }

static bool isAlphanumeric(const char c) {
  return isDecimal(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool isValidAlias(
    const char *alias) // useful if you have aliases defined in /etc/hosts
{
  size_t len = strlen(alias);

  if (len >= 255)
    return false;

  for (size_t i = 0; i < len; i++) {
    if (!isAlphanumeric(alias[i]) && alias[i] != '-' &&
        alias[i] != '_') // you can also have '.' in the name, but such an alias
                         // would pass the isDomain check the '_' character is
                         // also accepted, but not recommended
      return false;
  }

  return true;
}

static bool isValidDomain(const char *domain) {
  size_t len = strlen(domain);
  uint8_t subdomain = 0;
  bool dot = false;

  if (len > 253 || len < 4) { // mimimum domain name length is 4, I believe
                              // (example domain: a.co)
    return false;
  }

  if (domain[0] == '-' || domain[len - 1] == '-')
    return false;

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

static bool isValidHost(const char *ip) // checks if it's a valid IP(v4 or v6),
                                        // domain names are also accepted
{
  struct in_addr ipv4_addr;
  struct in6_addr ipv6_addr;

  return inet_pton(AF_INET, ip, &ipv4_addr) == 1 ||
         inet_pton(AF_INET6, ip, &ipv6_addr) == 1 || isValidDomain(ip) ||
         isValidAlias(ip);
}

static bool isValidChannelName(const char *channel_name) {
  if (channel_name == NULL ||
      *channel_name == '\0') // channel name can't be null
    return false;

  while (*channel_name != '\0') {
    if (!isAlphanumeric(*channel_name) && *channel_name != '/' &&
        *channel_name != '_')
      return false;
    channel_name++;
  }

  return true;
}

static int isValidPort(const char *port) {
  if (port == NULL || *port == '\0') // port can be null
    return 0;

  uint32_t value = 0;

  while (*port != '\0') {
    if (!isDecimal(*port))
      return -1;
    value = value * 10 + (*port - '0');
    port++;
  }

  if (value < 1024 || value > 65565)
    return -1;

  return value;
}

tlm_t tlm_open(uint8_t type, const char *channel_name, const char *ip,
               const char *port) {
  tlm_t new_tlm = {-1};
  new_tlm.type = type;
  uint32_t port_value;

  if (type == TLM_CLOSED) {
    errMsg("Can't open channel with type TLM_CLOSED. Valid types: "
           "TLM_PUBLISHER, TLM_SUBSCRIBER, TLM_BOTH.");
    return new_tlm;
  }

  if (type != TLM_BOTH && type != TLM_PUBLISHER && type != TLM_SUBSCRIBER) {
    errMsg("Invalid channel type. Can be TLM_BOTH, TLM_PUBLISHER or "
           "TLM_SUBSCRIBER.");
    return new_tlm;
  }

  if (!isValidHost(ip)) {
    errMsg(
        "Invalid host. Expected input is an IPv4 address of the form:'n.n.n.n' where n is a 8 bit unsigned value OR\n \ 
                                    an IPv6 address of the form:'hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh' where h is a hexadecimal value OR\n \
                                    a host OR\n \
                                    a domain name.");
    return new_tlm;
  }

  if ((port_value = isValidPort(port)) == -1) {
    errMsg("Invalid port. Port value can be anything between 1024 and "
           "65565(both inclusive).");
    return new_tlm;
  }

  if (!isValidChannelName(channel_name)) {
    errMsg("Invalid channel name. Must be a valid absolute UNIX path.");
    return new_tlm;
  }

  size_t n = strlen(channel_name);

  if (n > MAX_PATH_LENGTH) {
    errMsg("Path name is too long.");
    return new_tlm;
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

  if (s != 0) {
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
  uint8_t opcode = 2;
  uint8_t channel_len = (uint8_t)n;

  if (writen(sfd, &opcode, 1) < 0) {
    errMsg("Connection failed. Could not send request type to server.");
    new_tlm.sfd = -1;
    return new_tlm;
  }

  if (writen(sfd, &channel_len, 1) < 0) {
    errMsg("Connection failed. Could not send path length to server.");
    new_tlm.sfd = -1;
    return new_tlm;
  }

  if (writen(sfd, channel_name, channel_len) < 0) {
    errMsg("Connection failed. Could not send path to server.");
    new_tlm.sfd = -1;
    return new_tlm;
  }

  if (readn(sfd, &new_tlm.chn, 4) < 0) {
    errMsg("Connection failed. Could not connect to channel.");
    new_tlm.sfd = -1;
  }

  return new_tlm;
}

int32_t tlm_callback(tlm_t token,
                     void (*message_callback)(tlm_t token,
                                              const char *message)) {
  if (token.type == TLM_CLOSED) {
    errMsg("Token is closed.");
    return -1;
  }
}

const char *tlm_read(tlm_t token, uint32_t *message_id) {
  if (token.type == TLM_PUBLISHER) {
    errMsg("Can't read from channel: opened for message publishing.");
    return NULL;
  }

  if (token.type == TLM_CLOSED) {
    errMsg("Token is closed.");
    return NULL;
  }

  uint8_t opcode = 1;
  uint16_t message_length = 0;
  char *message;
  if (writen(token.sfd, &opcode, 1) < 0) {
    errMsg("Could not pass read request to server.");
    return NULL;
  }

  if (writen(token.sfd, &token.chn, 4) < 0) {
    errMsg("Could not pass channel token to server.");
    return NULL;
  }

  if (readn(token.sfd, &message_length, 2) < 0) {
    errMsg("Server did not respond.");
    return NULL;
  }

  printf("message_length is: %d\n", message_length);
  message = (char *)malloc((message_length + 1) * sizeof(char));
  message[message_length] = '\0';

  if (readn(token.sfd, message, message_length) < 0) {
    return NULL;
  }

  printf("received message from server: %s\n", message);
  return message;
}

int tlm_post(tlm_t token, const char *message) {
  if (token.type == TLM_SUBSCRIBER) {
    errMsg("Can't post to channel: opened for reading.");
    return -1;
  }

  if (token.type == TLM_CLOSED) {
    errMsg("Token is closed.");
    return -1;
  }

  size_t message_length = strlen(message);

  if (message_length >= MAX_MESSAGE_LENGTH) {
    return -1;
  }

  uint16_t msg_len = message_length;
  uint8_t opcode = 0;

  // send information to server
  if (writen(token.sfd, &opcode, 1) < 0) {
    errMsg("Connection failed. Could not send operation type to server.");
    return -1;
  }

  if (writen(token.sfd, &token.chn, 4) < 0) {
    errMsg("Connection failed. Could not send channel token to server.");
    return -1;
  }

  if (writen(token.sfd, &msg_len, 2) < 0) {
    errMsg("Connection failed. Could not send message length to server.");
    return -1;
  }

  if (writen(token.sfd, message, msg_len) < 0) {
    errMsg("Connection failed. Could not send message to server.");
    return -1;
  }

  // char* message_from_server = (char*)malloc((message_length + 1) *
  // sizeof(char)); if (readn(token.sfd, message_from_server, message_length) <
  // 0) {
  //     free(message_from_server);
  //     return -1;
  // }
  printf("Server received the message\n");

  // free(message_from_server);
  return 0;
}

void tlm_close(tlm_t token) {
  if (token.type == TLM_CLOSED) {
    errMsg("Token is already closed.");
    return;
  }

  uint8_t opcode = 3;
  if (writen(token.sfd, &opcode, 1) < 0) {
    errMsg("Could not send channel close request to server.");
  }

  if (writen(token.sfd, &token.chn, 4) < 0) {
    errMsg("Could not send channel close request to server.");
  }

  if (close(token.sfd) == -1) {
    errMsg("close");
  }
  token.type = TLM_CLOSED;
}

int tlm_type(tlm_t token) { return token.type; }

int main() {
  tlm_t tlm1 = tlm_open(
      TLM_BOTH,
      "/home/tpopescu/Documents/Workspace/Facultate/SO/Proiect_SO/channel_a",
      "localhost", DAEMON_DEFAULT_PORT);
  // tlm_close(tlm1);
  // tlm_t tlm2 = tlm_open(TLM_BOTH, "channel_a", "localhost",
  // DAEMON_DEFAULT_PORT); tlm_close(tlm2);
  tlm_post(tlm1, "Hello World!");
  // tlm_post(tlm1, "Hello World");
  tlm_read(tlm1, NULL);
  // tlm_close(tlm1);
  // tlm_post(tlm1, "Hello World2");
  // tlm_read(tlm1, NULL);

  tlm_close(tlm1);
}
