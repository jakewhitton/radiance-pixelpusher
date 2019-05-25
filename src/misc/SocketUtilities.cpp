#include "SocketUtilities.h"
#include <endian.h>
#include <poll.h>
#include <stdexcept>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include "misc/Log.h"
#include "config.h"

/**
 * Either sends or recvs ${totalBytes} bytes from sockfd, depending on parameters.  Terminates when terminate is detected to be true.
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
