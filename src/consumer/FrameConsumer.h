#ifndef FRAMECONSUMER_H
#define FRAMECONSUMER_H

#include "misc/Queue.h"
#include "dancefloor/Frame.h"

class FrameConsumer
{
public:
	virtual void consumeFrames(Queue<Frame> & frameQueue) = 0;
	virtual void stop() = 0;
};

#endif
