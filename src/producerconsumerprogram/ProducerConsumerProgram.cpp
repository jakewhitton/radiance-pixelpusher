#include "ProducerConsumerProgram.h"

using std::thread;

ProducerConsumerProgram::ProducerConsumerProgram(FrameProducer & producer, FrameConsumer & consumer)
	: _producer(producer)
	, _consumer(consumer)
{ }

void ProducerConsumerProgram::start()
{
	_producerThread = thread([&]
	{
		_producer.produceFrames(_queue);
	});
	
	_consumerThread = thread([&]
	{
		_consumer.consumeFrames(_queue);
	});
}

void ProducerConsumerProgram::stop()
{
	_producer.stop();
	_consumer.stop();

	_producerThread.join();
	_consumerThread.join();
}
