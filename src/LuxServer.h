#ifndef LUXSERVER_H
#define LUXSERVER_H

#include "BlockingCollection.h"
#include "Frame.h"
#include "RadianceRequestHandler.h"
#include <memory>
#include <thread>

class LuxServer
{
	using queue_t = code_machina::BlockingQueue<Frame>;
	
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
