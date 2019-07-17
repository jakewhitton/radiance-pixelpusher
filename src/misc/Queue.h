#ifndef QUEUE_H
#define QUEUE_H

#include "BlockingCollection.h"
#include "OperationInterruptedException.h"

template <typename Element>
class Queue
{
	using milliseconds = std::chrono::milliseconds;
	using Status = code_machina::BlockingCollectionStatus;
	
	code_machina::BlockingQueue<Element> _queue;

	static constexpr milliseconds timeout = milliseconds(10);
public:
	Queue(const int queueSize = 10)
		: _queue(queueSize)
	{ }

	void add(Element & e, const bool & condition, const bool trigger = true)
	{
		Status status;

		do
		{
			if (condition == trigger)
			{
				throw OperationInterruptedException();
			}

			status = _queue.try_add_timed(e, timeout);
		}
		while (status != Status::Ok);
	}

	void take(Element & e, const bool & condition, const bool trigger = true)
	{
		Status status;

		do
		{
			if (condition == trigger)
			{
				throw OperationInterruptedException();
			}

			status = _queue.try_take(e, timeout);
		}
		while (status != Status::Ok);
	}
};

#endif
