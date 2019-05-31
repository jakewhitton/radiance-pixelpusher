#include "Radiance.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <exception>
#include "misc/Log.h"
#include "misc/SocketUtilities.h"

/*
 * All of the socket code was adapted from Beejus' guide on network programming
 * in C, which is located at:
 *
 * http://beej.us/guide/bgnet/html/multi/
 */

using std::thread;
using std::exception;
using std::unique_ptr;

Radiance::Radiance(const char * port)
	: _serversockfd(SocketUtilities::getSocket(SocketType::SERVER, Protocol::TCP, "localhost", port))
	, _requestHandlerShouldTerminate(true)
	, _requestHandler(nullptr)
{
	// Make socket non-blocking so that we can check terminationg conditions while waiting
	// for a connection to be made to the server
	fcntl(_serversockfd, F_SETFL, O_NONBLOCK);
}

Radiance::~Radiance()
{
	close(_serversockfd);
}

void Radiance::produceFrames(FrameQueue & frameQueue)
{
	while (true)
	{
		// Accept a connection from radiance
		//
		// Note: the socket is configured to be non-blocking.
		// That means that calls to accept will fail with EAGAIN or EWOULDBLOCK if a
		// connection isn't available.  This is so we can continuously check exit condition.
		sockaddr connectionInfo;
		socklen_t bytesWritten;
		INFO("Waiting for radiance to connect...");
		int sockfd = accept(_serversockfd, &connectionInfo, &bytesWritten);
		while (sockfd == -1)
		{
			// If termination condition happened while waiting for a connection, terminate
			if (false)
			{
				if (_requestHandlerThread.joinable())
				{
					_requestHandlerShouldTerminate = true;
					_requestHandlerThread.join();
					_requestHandler = nullptr;
				}

				return;
			}

			sockfd = accept(_serversockfd, &connectionInfo, &bytesWritten);

			usleep(250000);
		}
		INFO("Radiance connected");

		// If paranoid about unauthorized clients connecting to the server, this would be
		// the place to place some kind of checks to ensure only specific types of clients
		// are able to connect.  This would be done by checking specific information in
		// connectionInfo (like IP address, source port, etc) and continuing to the next
		// loop iteration if the client isn't authorized.

		// If an old request handler was started, terminate it
		if (_requestHandlerThread.joinable())
		{
			_requestHandlerShouldTerminate = true;
			_requestHandlerThread.join();
			_requestHandler = nullptr;
		}

		// Add a new request handler
		_requestHandlerShouldTerminate = false;
		_requestHandler = std::make_unique<RequestHandler>(sockfd, frameQueue, _requestHandlerShouldTerminate);
		_requestHandlerThread = thread(std::ref(*_requestHandler));
	}
}

void Radiance::stop()
{

}
