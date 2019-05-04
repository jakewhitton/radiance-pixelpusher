#include "LuxServer.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "Log.h"

/*
 * All of the socket code was adapted from Beejus' guide on network programming
 * syscalls in C, which is located at:
 *
 * http://beej.us/guide/bgnet/html/multi/syscalls.html
 */

using code_machina::BlockingQueue;
using code_machina::BlockingCollectionStatus;

LuxServer::LuxServer(BlockingQueue<Frame> & queue)
	: _queue(queue)
{ }

Frame LuxServer::getNewFrame()
{
	return Frame::createFrame(0, 0, 255);
}

void LuxServer::operator()()
{
	BlockingCollectionStatus status;
	Frame frame = getNewFrame();

	while (!_queue.is_adding_completed())
	{
		status = _queue.try_add(frame);
		if (status == BlockingCollectionStatus::Ok)
		{
			frame = getNewFrame();
		}
	}
}
