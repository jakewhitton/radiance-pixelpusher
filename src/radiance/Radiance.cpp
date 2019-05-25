#include "Radiance.h"
#include "misc/SocketUtilities.h"
#include <cassert>

void Radiance::readRadianceMessage(const int sockfd, RadianceCommand * command, void * dataBuffer,
                                   const size_t dataBufferLength, const bool & terminate)
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
	SocketUtilities::recvAll(sockfd, &messageLengthLittleEndian, sizeof messageLengthLittleEndian, terminate);
	const uint32_t messageLength = SocketUtilities::littleEndianToHost(messageLengthLittleEndian);

	// Make sure data buffer is long enough to store message
	assert(dataBufferLength >= messageLength - 1);

	// Read command from socket
	uint8_t commandHolder;
	SocketUtilities::recvAll(sockfd, &commandHolder, sizeof commandHolder, terminate);
	*command = static_cast<RadianceCommand>(commandHolder);

	// Read data from socket
	SocketUtilities::recvAll(sockfd, dataBuffer, messageLength - 1, terminate);
}
