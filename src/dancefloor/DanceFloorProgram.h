#ifndef DANCEFLOORPROGRAM_H
#define DANCEFLOORPROGRAM_H

#include <thread>
#include "producer/FrameProducer.h"
#include "consumer/FrameConsumer.h"

using queue_t = code_machina::BlockingQueue<Frame>;

class DanceFloorProgram
{
	Queue<Frame> _queue;

	FrameProducer & _producer;
	std::thread _producerThread;

	FrameConsumer & _consumer;
	std::thread _consumerThread;

public:
	DanceFloorProgram(FrameProducer & producer, FrameConsumer & consumer);

	void start();
	void stop();
};

#endif
