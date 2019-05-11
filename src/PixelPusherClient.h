#ifndef PIXELPUSHERCLIENT_H
#define PIXELPUSHERCLIENT_H

#include "BlockingCollection.h"
#include "Frame.h"

class PixelPusherClient
{
	using queue_t = code_machina::BlockingQueue<Frame>;

	queue_t & _queue;
	const int _sockfd;

	static int getSocketFileDescriptor();

public:

	// Don't allow copying or moving
	PixelPusherClient(const PixelPusherClient &) = delete;
	PixelPusherClient(PixelPusherClient &&)      = delete;

	PixelPusherClient(queue_t & queue);
	~PixelPusherClient();

	void operator()();
};

#endif
