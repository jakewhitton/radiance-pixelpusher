#ifndef SOCKETUTILITIES_H
#define SOCKETUTILITIES_H

#include <exception>
#include <cstdint>

class SocketIOInterruptedException : public std::exception
{
	const char * what()
	{
		return "A socket I/O operation was aborted";
	}
};

class SocketUtilities
{
public:
	static void recvAll(const int sockfd, char * buffer, const size_t bytesToRead, const bool & terminate);

	static void sendAll(const int sockfd, const char * message, const size_t bytesToWrite, const bool & terminate);

	static void readRadianceMessage(const int sockfd, uint8_t * command, char * dataBuffer, const size_t dataBufferLength, const bool & terminate);

	static uint32_t hostToLittleEndian(uint32_t x);

	static uint32_t littleEndianToHost(uint32_t x);
};

#endif
