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

	// Write strip indices into message buffer (which won't change) and
	// populate data structure with the location of the actual color data
	// for each strip (which also won't change).
	//
	// Note: this code assumes each strip has the same number of pixels
	constexpr int stripSize = ddf.numberOfPixels() / ddf.numberOfStrips();
	for (int i = 0; i < ddf.numberOfStrips(); ++i)
	{
		auto stripIndexLocation = (uint8_t *)(_messageBuffer +
		                                      sequenceSize +
		                                      i * (stripSize + 1));
		*stripIndexLocation = i;

		_stripLocations[i] = (uint8_t (*)[3])(stripIndexLocation + 1); // Color data starts after index
	}
}

PixelPusher::~PixelPusher()
{
	close(_sockfd);
}

void PixelPusher::writeFrameDataToMessageBuffer(Frame & frame)
{
	auto frameData = (uint8_t (*)[ddf.height()][3])frame.data();
	for (const auto && pixelInfo : ddf)
	{
		auto strip = _stripLocations[pixelInfo.stripNumber];

		const PixelLocation & pos = pixelInfo.pos;
		strip[pos.i][0] = frameData[pos.x][pos.y][0];
		strip[pos.i][1] = frameData[pos.x][pos.y][1];
		strip[pos.i][2] = frameData[pos.x][pos.y][2];
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

		writeFrameDataToMessageBuffer(frame);

		int bytesWritten = send(_sockfd, _messageBuffer, sizeof _messageBuffer, 0);

		if (bytesWritten == -1)
		{
			ERR("Couldn't write to socket: %s", strerror(bytesWritten));
			exit(1);
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
