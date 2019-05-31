#ifndef FRAMEPRODUCER_H
#define FRAMEPRODUCER_H

#include "dancefloor/FrameQueue.h"

class FrameProducer
{
public:
	virtual void produceFrames(FrameQueue & frameQueue) = 0;
	virtual void stop() = 0;
};

#endif
