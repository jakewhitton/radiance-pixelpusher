#include "SocketUtilities.h"
#include <endian.h>
#include <poll.h>
#include <stdexcept>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include "misc/Log.h"
#include "config.h"

/*
 * All of the socket code was adapted from Beejus' guide on network programming in C, which is located at:
 *
 * http://beej.us/guide/bgnet/html/multi/
 */

int SocketUtilities::getSocket(const SocketType socketType, const Protocol protocol, const char * location, const char * port)
{
	int status;

	// Initialize hints
	addrinfo hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;    // No preference between IPv4 and IPv6
        hints.ai_socktype = (protocol == Protocol::TCP) ? SOCK_STREAM : SOCK_DGRAM;

	addrinfo * getaddrinfoResults;

	// Find available IP connection candidates for hostname
        status = getaddrinfo(location, port, &hints, &getaddrinfoResults);
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

		switch (socketType)
		{
		case SocketType::CLIENT:
		{
			// Try to connect
			status = connect(sockfd, entry->ai_addr, entry->ai_addrlen);
			if (status == -1)
			{
				// This socket couldn't connect; close it and try another
				close(sockfd);
				sockfd = -1;
				continue;
			}
		}
		case SocketType::SERVER:
		{
			// Try to bind to port
			status = bind(sockfd, entry->ai_addr, entry->ai_addrlen);
			if (status == -1) 
			{
				// This socket couldn't bind; close it and try another
				close(sockfd);
				sockfd = -1; 
				continue;
			}
			INFO("Connecting to port %s", port);

			// Try to listen on port
			const int connectionQueueSize = 10; // configures max number of pending connection requests
			status = listen(sockfd, connectionQueueSize);
			if (status == -1) 
			{
				// This socket couldn't listen; close it and try another
				close(sockfd);
				sockfd = -1; 
				continue;
			}
		}
		}

		// Socket was configured correctly
		break;
	}

	// Free linked list of IP connection candidates for hostname
	freeaddrinfo(getaddrinfoResults);

	if (sockfd == -1)
	{
		ERR("Could not get %s %s socket for %s:%s", (socketType == SocketType::SERVER) ? "server" : "client",
		                                            (protocol == Protocol::TCP) ? "TCP" : "UDP",
							    location,
							    port);
		exit(1);
	}

	return sockfd;
}

/**
 * Either sends or recvs ${totalBytes} bytes from ${sockfd}, depending on parameters.  Terminates when ${terminate} is detected to be true.
 *
 * @param sockfd the socket file descriptor that this socket I/O operation is acting on
 * @param buffer a pointer to a buffer of at least ${totalBytes} bytes, should be nullptr if operation is a send
 * @param message a pointer to a message that is at least ${totalBytes} bytes long, should be nullptr if operation is a recv
 * @param totalBytes the size of the message to be sent or recv'd
 * @param terminate a reference to a boolean that will be read frequently; if terminate is read as true, a SocketIOInterruptedException is thrown
 */
static void repeatUntilFullyProcessed(const int sockfd, void * buffer, const void * message, const size_t totalBytes, const bool & terminate);

void SocketUtilities::recvAll(const int sockfd, void * buffer, const size_t bytesToRead, const bool & terminate)
{
	repeatUntilFullyProcessed(sockfd, buffer, nullptr, bytesToRead, terminate);
}

void SocketUtilities::sendAll(const int sockfd, const void * message, const size_t bytesToWrite, const bool & terminate)
{
	repeatUntilFullyProcessed(sockfd, nullptr, message, bytesToWrite, terminate);
}

uint32_t SocketUtilities::hostToLittleEndian(uint32_t x)
{
	// [h]ost [to] [l]ittle [e]ndian [32]-bit - htole32
	return htole32(x);
}

uint32_t SocketUtilities::littleEndianToHost(uint32_t x)
{
	// [l]ittle [e]ndian [32]-bit [to] [h]ost - le32toh
	return le32toh(x);
}

enum SocketOperation
{
	RECV,
	SEND
};

// Adapted from:
// https://stackoverflow.com/questions/13479760/c-socket-recv-and-send-all-data
static void repeatUntilFullyProcessed(const int sockfd, void * buffer, const void * message, const size_t totalBytes, const bool & terminate)
{
	// Ensure that one and exactly one of buffer and message are populated
	if ((buffer && message) || (!buffer && !message))
	{
		ERR("Socket I/O operation that is neither a send nor a recv.");
		exit(1);
	}

	// buffer is populated if a recv is being performed; otherwise, it's a send
	const SocketOperation op = buffer ? RECV : SEND;

	// Set up polling information
	const int numberOfFileDescriptors = 1;
	const short int requestedEvent = (op == RECV) ?
	                                 POLLIN   // Poll for available input data
				         :
				         POLLOUT; // Poll for space for outgoing data
	pollfd pollFileDescriptors[numberOfFileDescriptors] {{sockfd, requestedEvent, 0}};

	size_t bytesDone = 0;

	while (bytesDone < totalBytes)
	{
		int newBytes = -1;

		do
		{
			// Poll on sockfd to wait for I/O availability
			int status = poll(pollFileDescriptors, numberOfFileDescriptors, frameTime);
			if (status == -1)
			{
				ERR("Failed to poll socket %d: %s", sockfd, strerror(errno));
			}

			// If sockfd is ready for operation, perform corresponding action
			if (pollFileDescriptors[0].revents & requestedEvent)
			{
				const int remainingBytes = totalBytes - bytesDone;
				newBytes = (op == RECV) ? 
				           recv(sockfd, (char *)buffer + bytesDone,  remainingBytes, 0)
					   :
					   send(sockfd, (char *)message + bytesDone, remainingBytes, 0);
			}
			else if (terminate)
			{
				throw SocketIOInterruptedException();
			}
		}
		while (newBytes == -1);

		bytesDone += newBytes;
	}
}
