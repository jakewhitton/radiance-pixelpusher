#include "FrameQueue.h"

using code_machina::BlockingCollectionStatus;

FrameQueue::FrameQueue()
	: _queue(QUEUE_SIZE)
{ }

bool FrameQueue::add(Frame & frame, std::chrono::milliseconds timeout)
{
	auto status = _queue.try_add_timed(std::move(frame), timeout);

	return status == BlockingCollectionStatus::Ok;
}

bool FrameQueue::take(Frame & frame, std::chrono::milliseconds timeout)
{
	auto status = _queue.try_take(frame, timeout);

	return status == BlockingCollectionStatus::Ok;
}
