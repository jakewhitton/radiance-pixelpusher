#ifndef RADIANCEREQUESTHANDLER_H
#define RADIANCEREQUESTHANDLER_H

#include "BlockingCollection.h"
#include "Frame.h"

class RadianceRequestHandler
{
	using queue_t = code_machina::BlockingQueue<Frame>;

	queue_t & _queue;
	const int _sockfd;
	bool & _shouldTerminate;

	//void sendLookupCoordinates2D();
	//void sendGetFrame(uint32_t delay);
	//void getAndPushFrames();

public:
	
	// Don't allow copying or moving
	RadianceRequestHandler(const RadianceRequestHandler &) = delete;
	RadianceRequestHandler(RadianceRequestHandler &&) = delete;

	RadianceRequestHandler(const int sockfd, queue_t & queue, bool & shouldTerminate);
	~RadianceRequestHandler();

	void operator()();
};

#endif
