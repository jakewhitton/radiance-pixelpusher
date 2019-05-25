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
	static void recvAll(const int sockfd, void * buffer, const size_t bytesToRead, const bool & terminate);

	static void sendAll(const int sockfd, const void * message, const size_t bytesToWrite, const bool & terminate);

	static uint32_t hostToLittleEndian(uint32_t x);

	static uint32_t littleEndianToHost(uint32_t x);
};

#endif
