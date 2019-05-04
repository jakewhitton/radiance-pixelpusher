#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void printInfo(const addrinfo * addrinfo);

void testSendTo(const int sockfd, const addrinfo * addrinfo);
