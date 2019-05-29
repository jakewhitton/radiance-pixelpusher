#ifndef LUXSERVER_H
#define LUXSERVER_H

#include <memory>
#include <thread>
#include "producer/FrameProducer.h"
#include "RadianceRequestHandler.h"

class LuxServer
	: public FrameProducer
{
	const int _serversockfd;

	// State to limit active clients to one at a time
	bool _requestHandlerShouldTerminate;
	std::unique_ptr<RadianceRequestHandler> _requestHandler;
	std::thread _requestHandlerThread;

public:

	// Don't allow copying or moving
	LuxServer(const LuxServer &) = delete;
	LuxServer(LuxServer &&)      = delete;

	LuxServer(const char * port);
	~LuxServer();

	void produceFrames(FrameQueue & frameQueue) override;
	void stop() override;
};

#endif
