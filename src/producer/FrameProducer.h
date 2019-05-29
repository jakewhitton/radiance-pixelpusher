#ifndef FRAMEPRODUCER_H
#define FRAMEPRODUCER_H

#include "producerconsumerprogram/FrameQueue.h"

class FrameProducer
{
public:
	virtual void produceFrames(FrameQueue & frameQueue) = 0;
	virtual void stop() = 0;
};

#endif
