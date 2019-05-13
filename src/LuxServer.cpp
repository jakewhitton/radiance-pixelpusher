#include "LuxServer.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <exception>
#include "Log.h"
#include "RadianceRequestHandler.h"

#include "helper.h"

/*
 * All of the socket code was adapted from Beejus' guide on network programming
 * in C, which is located at:
 *
 * http://beej.us/guide/bgnet/html/multi/
 */

using std::thread;
using std::exception;
using std::unique_ptr;

LuxServer::LuxServer(queue_t & queue)
	: _queue(queue)
	, _serversockfd(getSocketFileDescriptor())
	, _requestHandlerShouldTerminate(true)
	, _requestHandler(nullptr)
{ }

LuxServer::~LuxServer()
{
	close(_serversockfd);
}

void LuxServer::operator()()
{
	while (!_queue.is_adding_completed())
	{
		// Accept a connection from radiance
		//
		// Note: in getSocketFileDescriptor(), the socket is configured to be non-blocking.
		// That means that calls to accept will fail with EAGAIN or EWOULDBLOCK if a
		// connection isn't available.  This is so we can continuously check exit condition.
		sockaddr connectionInfo;
		socklen_t bytesWritten;
		INFO("Waiting for radiance to connect to localhost:%s", LUXSERVER_PORT);
		int sockfd = accept(_serversockfd, &connectionInfo, &bytesWritten);
		while (sockfd == -1)
		{
			// If termination condition happened while waiting for a connection, terminate
			if (_queue.is_adding_completed())
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
		_requestHandler = unique_ptr<RadianceRequestHandler> {new RadianceRequestHandler(sockfd, _queue, _requestHandlerShouldTerminate)};
		_requestHandlerThread = thread(std::ref(*_requestHandler));
	}
}

int LuxServer::getSocketFileDescriptor()
{
        int status;

        // Initialize hints
        addrinfo hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;     // No preference between IPv4 and IPv6
        hints.ai_socktype = SOCK_STREAM; // Stream (TCP) socket, what radiance expects
	hints.ai_flags = AI_PASSIVE;     // Listen on localhost

        addrinfo * getaddrinfoResults;

        // Find available IP connection candidates for hostname
        status = getaddrinfo("localhost", LUXSERVER_PORT, &hints, &getaddrinfoResults);
        if (status != 0)
	{
            ERR("Failed getaddrinfo: %s", gai_strerror(status));
            exit(1);
        }

        // Obtain socket file descriptor that is listening on the correct port
        int sockfd = -1;
        for (addrinfo * entry = getaddrinfoResults; entry != nullptr; entry = entry->ai_next)
        {
                // Get file descriptor
                sockfd = socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
                if (sockfd == -1)
                {
                        continue;
                }

                // Try to bind to port
                status = bind(sockfd, entry->ai_addr, entry->ai_addrlen);
                if (status == -1)
                {
                        // This socket couldn't bind; close it and try another
                        close(sockfd);
                        sockfd = -1;
                        continue;
                }
		INFO("Connecting to port %s", LUXSERVER_PORT);

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

		printInfo(entry);

                // Socket was created and is listening on port; stop trying
                break;
	}

        // Free linked list of IP connection candidates for hostname
        freeaddrinfo(getaddrinfoResults);

        if (sockfd == -1)
        {
                ERR("Could not listen on port %s", LUXSERVER_PORT);
                exit(1);
        }

	// Make socket non-blocking so that we can check terminationg conditions while waiting
	// for a connection to be made to the server
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

        return sockfd;
}
