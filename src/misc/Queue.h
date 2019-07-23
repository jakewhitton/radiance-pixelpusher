#ifndef QUEUE_H
#define QUEUE_H

#include "BlockingCollection.h"
#include "OperationInterruptedException.h"
#include "misc/Log.h"

template <typename Element>
class Queue
{
	using milliseconds = std::chrono::milliseconds;
	using Status = code_machina::BlockingCollectionStatus;
	
	code_machina::BlockingQueue<Element> _queue;

	static constexpr milliseconds defaultTimeout = milliseconds(10);
public:
	Queue(const int queueSize = 10)
		: _queue(queueSize)
	{ }

	bool empty()
	{
		return _queue.empty();
	}

	void add(Element & e, const bool & condition, const bool trigger = true)
	{
		Status status;
		do
		{
			if (condition == trigger)
			{
				throw OperationInterruptedException();
			}

			status = _queue.try_add_timed(e, defaultTimeout);
		}
		while (status != Status::Ok);
	}

	bool add(Element & e, unsigned timeoutMs)
	{
		Status status = _queue.try_add_timed(e, milliseconds(timeoutMs));
		
		return status == Status::Ok;
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

			status = _queue.try_take(e, defaultTimeout);
		}
		while (status != Status::Ok);
	}

	bool take(Element & e, unsigned timeoutMs)
	{
		Status status = _queue.try_take(e, milliseconds(timeoutMs));
		
		return status == Status::Ok;
	}
};

#endif
