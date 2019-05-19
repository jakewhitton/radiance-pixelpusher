#include "SocketUtilities.h"
#include <endian.h>
#include <poll.h>
#include <stdexcept>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include "Log.h"

// Adapted from:
// https://stackoverflow.com/questions/13479760/c-socket-recv-and-send-all-data
void SocketUtilities::recvAll(const int sockfd, char * buffer, const size_t bytesToRead, const bool & terminate)
{
	// Set up polling information
	const int numberOfFileDescriptors = 1;
	const int requestedEvents = POLLIN; // Poll for input
	pollfd pollFileDescriptors[numberOfFileDescriptors] {{sockfd, requestedEvents, 0}};

	size_t remainingBytesToRead = bytesToRead;

	while (remainingBytesToRead > 0)
	{
		int newBytesRead = -1;

		do
		{
			// Poll on sockfd to wait for input
			const int timeoutMilliseconds = 100;
			int status = poll(pollFileDescriptors, numberOfFileDescriptors, timeoutMilliseconds);
			if (status == -1)
			{
				ERR("Failed to poll socket %d: %s", sockfd, strerror(errno));
			}

			// If input is ready, read from socket
			if (pollFileDescriptors[0].revents & POLLIN)
			{
				newBytesRead = recv(sockfd, buffer, remainingBytesToRead, 0);
			}
			else if (terminate)
			{
				throw SocketIOInterruptedException();
			}
		}
		while (newBytesRead == -1);

		remainingBytesToRead -= newBytesRead;
		buffer += newBytesRead;
	}
}

// Adapted from:
// https://stackoverflow.com/questions/13479760/c-socket-recv-and-send-all-data
void SocketUtilities::sendAll(const int sockfd, const char * message, const size_t bytesToWrite, const bool & terminate)
{
	// Set up polling information
	const int numberOfFileDescriptors = 1;
	const int requestedEvents = POLLOUT; // Poll for output
	pollfd pollFileDescriptors[numberOfFileDescriptors] {{sockfd, requestedEvents, 0}};

	size_t remainingBytesToWrite = bytesToWrite;

	while (remainingBytesToWrite > 0)
	{
		int newBytesWritten = -1;

		do
		{
			// Poll on sockfd to wait for input
			const int timeoutMilliseconds = 100;
			int status = poll(pollFileDescriptors, numberOfFileDescriptors, timeoutMilliseconds);
			if (status == -1)
			{
				ERR("Failed to poll socket %d: %s", sockfd, strerror(errno));
			}

			// If socket can accept output, write
			if (pollFileDescriptors[0].revents & POLLOUT)
			{
				newBytesWritten = send(sockfd, message, remainingBytesToWrite, 0);
			}
			else if (terminate)
			{
				throw SocketIOInterruptedException();
			}
		}
		while (newBytesWritten == -1);

		remainingBytesToWrite -= newBytesWritten;
		message += newBytesWritten;
	}
}

void SocketUtilities::readRadianceMessage(const int sockfd, uint8_t * command, char * dataBuffer, const size_t dataBufferLength, const bool & terminate)
{
	/*
	 * Radiance messages have a specific format, which can be found at:
	 *
	 * https://github.com/zbanks/radiance/blob/master/light_output.md
	 *
	 * The format for a message is as follows:
	 *
	 * +---------------------------------------------------------------+
	 * | Length (4 bytes) | Command (1 byte) | Data (Length - 1 bytes) |
	 * +---------------------------------------------------------------+
	 *
	 * Note that length (and any other integral values longer than one byte) are assumed to
	 * be little endian unless otherwise specified, so you must convert them to host endianness.
	 */

	// Read length from socket so we know how many bytes to read for rest of message
	uint32_t messageLengthLittleEndian;
	recvAll(sockfd, (char *)&messageLengthLittleEndian, sizeof messageLengthLittleEndian, terminate);
	const uint32_t messageLength = littleEndianToHost(messageLengthLittleEndian);

	// Make sure data buffer is long enough to store message
	assert(dataBufferLength >= messageLength - 1);

	// Read command from socket
	recvAll(sockfd, (char *)command, sizeof (uint8_t), terminate);

	// Read data from socket
	recvAll(sockfd, dataBuffer, messageLength - 1, terminate);
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
