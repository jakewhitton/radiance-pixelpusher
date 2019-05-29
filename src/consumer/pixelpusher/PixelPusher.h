#ifndef PIXELPUSHER_H
#define PIXELPUSHER_H

#include "consumer/FrameConsumer.h"

class PixelPusher
	: public FrameConsumer
{
private:
	const int _sockfd;
	bool _running;

public:
	// Don't allow copying or moving
	PixelPusher(const PixelPusher &) = delete;
	PixelPusher(PixelPusher &&)      = delete;

	PixelPusher(const char * location, const char * port);
	~PixelPusher();

	void consumeFrames(FrameQueue & frameQueue) override;
	void stop() override;
};

#endif
