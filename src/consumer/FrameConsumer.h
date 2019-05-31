#ifndef FRAMECONSUMER_H
#define FRAMECONSUMER_H

#include "dancefloor/FrameQueue.h"

class FrameConsumer
{
public:
	virtual void consumeFrames(FrameQueue & frameQueue) = 0;
	virtual void stop() = 0;
};

#endif
