#include "Radiance.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>
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
	, _terminateServer(false)
	, _terminateRequestHandler(true)
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

void Radiance::stopRequestHandler()
{
	// A thread is not joinable if:
	//  - it was default-constructed
	//  - it was previously joined or detached
	if (_requestHandlerThread.joinable())
	{
		_terminateRequestHandler = true;
		_requestHandlerThread.join();
		_requestHandler = nullptr;
	}
}

void Radiance::produceFrames(FrameQueue & frameQueue)
{
	/*
	 * Note that it's important that any exit path from this function closes by calling stopRequestHandler()
	 *
	 * If this is not done, _requestHandlerThread will continue executing and accessing memory from objects
	 * that might be destructed at any time (specifically, it accesses a FrameQueue reference that originates
	 * from a DanceFloorProgram object and a reference to the boolean _terminateRequestHandler in this class).
	 */

	INFO("Waiting for radiance client to connect...");

	while (!_terminateServer)
	{
		// Accept a connection from radiance client
		int sockfd;
		try
		{
			sockfd = SocketUtilities::acceptConnection(_serversockfd, _terminateServer);
		}
		catch (OperationInterruptedException e)
		{
			// Possible termination point: 1
			stopRequestHandler();
			return;
		}

		INFO("Radiance client connected...");

		// TODO Implement security measure by adding client info reception to acceptConnection interface

		// If an old request handler was started, terminate it
		stopRequestHandler();

		// Add a new request handler
		_terminateRequestHandler = false;
		_requestHandler = std::make_unique<RequestHandler>(sockfd, frameQueue, _terminateRequestHandler);
		_requestHandlerThread = thread(std::ref(*_requestHandler));
	}

	// Possible termination point: 2
	stopRequestHandler();
}

void Radiance::stop()
{
	_terminateServer = true;
}
