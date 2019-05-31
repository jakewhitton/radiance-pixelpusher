#include "DanceFloorProgram.h"

using std::thread;

DanceFloorProgram::DanceFloorProgram(FrameProducer & producer, FrameConsumer & consumer)
	: _producer(producer)
	, _consumer(consumer)
{ }

void DanceFloorProgram::start()
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

void DanceFloorProgram::stop()
{
	_producer.stop();
	_consumer.stop();

	_producerThread.join();
	_consumerThread.join();
}
