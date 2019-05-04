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
 * syscalls in C, which is located at:
 *
 * http://beej.us/guide/bgnet/html/multi/syscalls.html
 */

using code_machina::BlockingQueue;

PixelPusherClient::PixelPusherClient(BlockingQueue<Frame> & queue)
	: _queue(queue)
{
	populateSockets();
}

PixelPusherClient::~PixelPusherClient()
{
	close(_sockfd);
	freeaddrinfo(getaddrinfoResults);
}

void PixelPusherClient::operator()()
{
	for (auto frame : _queue)
	{
		int bytesWritten = sendto(_sockfd, frame.data(), Frame::SIZE, 0, serverInfo->ai_addr, serverInfo->ai_addrlen);

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

void PixelPusherClient::populateSockets()
{
	// Initialize hints
	addrinfo hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        int status = getaddrinfo(PIXELPUSHER_LOCATION, PIXELPUSHER_PORT, &hints, &getaddrinfoResults);
        if (status != 0) {
            ERR("Failed getaddrinfo: %s", gai_strerror(status));
            exit(1);
        }

	int sockfd = -1;
	for (addrinfo * entry = getaddrinfoResults; entry != nullptr; entry = entry->ai_next)
	{
		sockfd = socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
		if (sockfd != 0)
		{
			_sockfd = sockfd;
			serverInfo = entry;
			break;
		}
	}

	if (sockfd == -1)
	{
		ERR("Could not obtain socket from %s:%s", PIXELPUSHER_LOCATION, PIXELPUSHER_PORT);
		exit(-1);
	}
}
