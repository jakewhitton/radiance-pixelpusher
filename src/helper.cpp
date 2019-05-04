#include "helper.h"
#include "Log.h"

/*
 * All of the socket code was adapted from Beejus' guide on network programming
 * syscalls in C, which is located at:
 *
 * http://beej.us/guide/bgnet/html/multi/syscalls.html
 */

void printInfo(const addrinfo * addrinfo)
{
	void * addr;

	if (addrinfo->ai_family == AF_INET) {
		sockaddr_in * sockaddr_ipv4 = (sockaddr_in *)addrinfo->ai_addr;
		addr = &(sockaddr_ipv4->sin_addr);
	}
	else if (addrinfo->ai_family == AF_INET6) {
		sockaddr_in6 * sockaddr_ipv6 = (sockaddr_in6 *)addrinfo->ai_addr;
		addr = &(sockaddr_ipv6->sin6_addr);
	} else {
		ERR("Unexpected addrinfo->ai_family...");
	}

	char ipstr[INET6_ADDRSTRLEN];
	inet_ntop(addrinfo->ai_family, addr, ipstr, sizeof ipstr);

	INFO("addrinfo->ai_addr: %s", ipstr);
	INFO("addrinfo->ai_addrlen: %d", addrinfo->ai_addrlen);
}

void testSendTo(const int sockfd, const addrinfo * addrinfo)
{
		char message[] = "hello, world!";
		int bytesWritten = sendto(sockfd, message, sizeof message, 0, addrinfo->ai_addr, addrinfo->ai_addrlen);
		INFO("%d bytes written to sockfd %d", bytesWritten, sockfd);
}
