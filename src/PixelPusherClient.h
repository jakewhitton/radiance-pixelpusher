#ifndef PIXELPUSHERCLIENT_H
#define PIXELPUSHERCLIENT_H

#include "BlockingCollection.h"
#include "Frame.h"
#include <netdb.h>

class PixelPusherClient
{
	code_machina::BlockingQueue<Frame> & _queue;
	int _sockfd; // Socket file descriptor
	addrinfo * getaddrinfoResults;
	const addrinfo * serverInfo;

	void populateSockets();

public:
	PixelPusherClient(code_machina::BlockingQueue<Frame> & queue);
	~PixelPusherClient();

	void operator()();
};

#endif
