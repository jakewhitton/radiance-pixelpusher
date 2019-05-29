#ifndef FRAMEQUEUE_H
#define FRAMEQUEUE_H

#include "BlockingCollection.h"
#include "Frame.h"

class FrameQueue
{
	static constexpr const int QUEUE_SIZE = 10;
	code_machina::BlockingQueue<Frame> _queue;

public:
	FrameQueue();

	bool  add(Frame & frame, std::chrono::milliseconds timeout);
	bool take(Frame & frame, std::chrono::milliseconds timeout);
};

#endif
