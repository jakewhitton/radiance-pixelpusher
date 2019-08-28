#include "PixelPusher.h"
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include "misc/SocketUtilities.h"
#include "misc/Log.h"

PixelPusher::PixelPusher(const char * location, const char * port)
	: _sockfd(SocketUtilities::getSocket(SocketType::CLIENT, Protocol::UDP, location, port))
	, _running(true)
{
	// Set sequence value in message buffer to 0
	auto sequenceLocation = (uint32_t *)_messageBuffer;
	*sequenceLocation = 0;
}

PixelPusher::~PixelPusher()
{
	close(_sockfd);
}

void PixelPusher::writeFrameDataToMessageBuffer(Frame & frame, int lowStrip, int highStrip)
{
	auto frameData = (uint8_t (*)[ddf.height()][3])frame.data();

	const PixelLocation origin = ddf.getOrigin();

	for (int strip = 0; strip <= highStrip - lowStrip; ++strip)
	{
		auto itr = ddf.begin() + (lowStrip + strip) * pixelsInStrip;
		auto end = highStrip == 5 ?
		           ddf.end()
			   :
			   (itr + pixelsInStrip);

		uint8_t * stripIndex = _messageBuffer + sequenceSize + strip * pixelsInStrip;
		*stripIndex = lowStrip + strip;

		auto stripData = (uint8_t (*)[3])(stripIndex + 1);

		for (; itr != end; ++itr)
		{
			const PixelInfo info = *itr;

			const int x = info.pos.x - origin.x;
			const int y = info.pos.y - origin.y;

			stripData[info.pos.i][0] = frameData[x][y][0];
			stripData[info.pos.i][1] = frameData[x][y][1];
			stripData[info.pos.i][2] = frameData[x][y][2];
		}
	}
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

		for (int strip = 0; strip < ddf.numberOfStrips(); strip += stripsInPacket)
		{
			//INFO("\n\n\nWriting strips %d - %d\n\n\n", strip, strip + stripsInPacket);
			writeFrameDataToMessageBuffer(frame, strip, std::min(strip + stripsInPacket, ddf.numberOfStrips()) - 1);

			int bytesWritten = send(_sockfd, _messageBuffer, sizeof _messageBuffer, 0);

			if (bytesWritten == -1)
			{
				ERR("Couldn't write to socket: %s", strerror(bytesWritten));
				exit(1);
			}

			INFO("%d bytes written", bytesWritten);

			const unsigned timeout = 1; // ms
			usleep(timeout * 1000);
		}

		// Increment sequence in message buffer
		auto sequenceLocation = (uint32_t *)_messageBuffer;
		++*sequenceLocation;
	}
}

void PixelPusher::stop()
{
	_running = false;
}
