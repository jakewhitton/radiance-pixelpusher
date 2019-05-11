#include "PixelPusherClient.h"
#include "Log.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "helper.h"

/*
 * All of the socket code was adapted from Beejus' guide on network programming
 * in C, which is located at:
 *
 * http://beej.us/guide/bgnet/html/multi/
 */

PixelPusherClient::PixelPusherClient(queue_t & queue)
	: _queue(queue)
	, _sockfd(getSocketFileDescriptor())
{ }

PixelPusherClient::~PixelPusherClient()
{
	close(_sockfd);
}

void PixelPusherClient::operator()()
{
	for (auto frame : _queue)
	{
		int bytesWritten = send(_sockfd, frame.data(), Frame::SIZE, 0);

		if (bytesWritten == -1)
		{
			ERR("Couldn't write to socket: %s", strerror(bytesWritten));
			exit(1);
		}
		else if (bytesWritten < Frame::SIZE)
		{
			ERR("Only wrote %d bytes / %d bytes", bytesWritten, Frame::SIZE);
			exit(1);
		}
	}
}

int PixelPusherClient::getSocketFileDescriptor()
{
	int status;

	// Initialize hints
	addrinfo hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;    // No preference between IPv4 and IPv6
        hints.ai_socktype = SOCK_DGRAM; // Datagram (UDP) socket, what the pixel pusher expects

	addrinfo * getaddrinfoResults;

	// Find available IP connection candidates for hostname
        status = getaddrinfo(PIXELPUSHER_LOCATION, PIXELPUSHER_PORT, &hints, &getaddrinfoResults);
        if (status != 0) {
            ERR("Failed getaddrinfo: %s", gai_strerror(status));
            exit(1);
        }

	// Obtain socket file descriptor that is connected to the pixel pusher
	int sockfd = -1;
	for (addrinfo * entry = getaddrinfoResults; entry != nullptr; entry = entry->ai_next)
	{
		// Get file descriptor
		sockfd = socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
		if (sockfd == -1)
		{
			continue;
		}

		// Try to connect
		status = connect(sockfd, entry->ai_addr, entry->ai_addrlen);
		if (status == -1)
		{
			// This socket couldn't connect; close it and try another
			close(sockfd);
			sockfd = -1;
			continue;
		}

		// Socket was created and is connected; stop trying
		break;
	}

	// Free linked list of IP connection candidates for hostname
	freeaddrinfo(getaddrinfoResults);

	if (sockfd == -1)
	{
		ERR("Could not connect to %s:%s", PIXELPUSHER_LOCATION, PIXELPUSHER_PORT);
		exit(1);
	}

	return sockfd;
}
