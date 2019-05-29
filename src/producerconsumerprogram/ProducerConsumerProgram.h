#ifndef PRODUCERCONSUMERPROGRAM_H
#define PRODUCERCONSUMERPROGRAM_H

#include <thread>
#include "producer/FrameProducer.h"
#include "consumer/FrameConsumer.h"
#include "FrameQueue.h"

using queue_t = code_machina::BlockingQueue<Frame>;

class ProducerConsumerProgram
{
	FrameQueue _queue;

	FrameProducer & _producer;
	std::thread _producerThread;

	FrameConsumer & _consumer;
	std::thread _consumerThread;

public:
	ProducerConsumerProgram(FrameProducer & producer, FrameConsumer & consumer);

	void start();
	void stop();
};

#endif
