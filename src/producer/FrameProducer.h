#ifndef FRAMEPRODUCER_H
#define FRAMEPRODUCER_H

#include "misc/Queue.h"
#include "dancefloor/Frame.h"

class FrameProducer
{
public:
	virtual void produceFrames(Queue<Frame> & frameQueue) = 0;
	virtual void stop() = 0;
};

#endif
