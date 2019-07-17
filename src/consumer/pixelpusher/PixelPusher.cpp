#include "PixelPusher.h"
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include "misc/SocketUtilities.h"
#include "misc/Log.h"

PixelPusher::PixelPusher(const char * location, const char * port)
	: _sockfd(SocketUtilities::getSocket(SocketType::CLIENT, Protocol::UDP, location, port))
	, _running(true)
{ }

PixelPusher::~PixelPusher()
{
	close(_sockfd);
}

void PixelPusher::consumeFrames(Queue<Frame> & frameQueue)
{
	Frame frame;

	while (_running)
	{
		try
		{
			frameQueue.take(frame, _running, false); // Throw when _running == false
		}
		catch (OperationInterruptedException e)
		{
			break;
		}

		int bytesWritten = send(_sockfd, frame.data(), frame.getSize(), 0);

		if (bytesWritten == -1)
		{
			ERR("Couldn't write to socket: %s", strerror(bytesWritten));
			exit(1);
		}
	}
}

void PixelPusher::stop()
{
	_running = false;
}
