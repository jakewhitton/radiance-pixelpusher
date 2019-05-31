#ifndef RADIANCE_H
#define RADIANCE_H

#include <memory>
#include <thread>
#include "producer/FrameProducer.h"
#include "RequestHandler.h"

class Radiance
	: public FrameProducer
{
	const int _serversockfd;

	// State to limit active clients to one at a time
	bool _requestHandlerShouldTerminate;
	std::unique_ptr<RequestHandler> _requestHandler;
	std::thread _requestHandlerThread;

public:

	// Don't allow copying or moving
	Radiance(const Radiance &) = delete;
	Radiance(Radiance &&)      = delete;

	Radiance(const char * port);
	~Radiance();

	void produceFrames(FrameQueue & frameQueue) override;
	void stop() override;
};

#endif
