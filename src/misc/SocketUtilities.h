#ifndef SOCKETUTILITIES_H
#define SOCKETUTILITIES_H

#include <exception>
#include <cstdint>

class OperationInterruptedException : public std::exception
{
	const char * what()
	{
		return "A socket I/O operation was aborted";
	}
};

enum class SocketType
{
	CLIENT,
	SERVER
};

enum class Protocol
{
	TCP,
	UDP
};

class SocketUtilities
{
public:
	static int getSocket(const SocketType socketType, const Protocol protocol, const char * location, const char * port);

	static int acceptConnection(const int serversockfd, const bool & terminate);

	static void recvAll(const int sockfd, void * buffer, const size_t bytesToRead, const bool & terminate);

	static void sendAll(const int sockfd, const void * message, const size_t bytesToWrite, const bool & terminate);

	static uint32_t hostToLittleEndian(uint32_t x);

	static uint32_t littleEndianToHost(uint32_t x);
};

#endif
