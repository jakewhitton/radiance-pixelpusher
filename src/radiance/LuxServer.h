#ifndef LUXSERVER_H
#define LUXSERVER_H

#include "pixelpusher/PixelPusherClient.h"
#include "RadianceRequestHandler.h"
#include <memory>
#include <thread>

class LuxServer
{
	using queue_t = PixelPusherClient::queue_t;
	
	queue_t & _queue;
	const int _serversockfd;

	// State to limit active clients to one at a time
	bool _requestHandlerShouldTerminate;
	std::unique_ptr<RadianceRequestHandler> _requestHandler;
	std::thread _requestHandlerThread;

	static int getSocketFileDescriptor();

public:

	// Don't allow copying or moving
	LuxServer(const LuxServer &) = delete;
	LuxServer(LuxServer &&)      = delete;

	LuxServer(queue_t & queue);
	~LuxServer();

	void operator()();
};

#endif
