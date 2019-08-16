#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include "misc/Queue.h"
#include "dancefloor/Frame.h"
#include "ddf.h"

class RequestHandler
{
	Queue<Frame> & _queue;
	const int _sockfd;
	const bool & _terminate;

	uint8_t _rgbaBuffer[ddf.width()][ddf.height()][4];

	void sendLookupCoordinates2D();
	void sendGetFrame(uint32_t delay);
	void getAndPushFrames();

public:
	
	// Don't allow copying or moving
	RequestHandler(const RequestHandler &) = delete;
	RequestHandler(RequestHandler &&)      = delete;

	RequestHandler(const int sockfd, Queue<Frame> & queue, const bool & terminate);
	~RequestHandler();

	void operator()();
};

#endif
