#include "RadianceRequestHandler.h"
#include "config.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include "Log.h"

RadianceRequestHandler::RadianceRequestHandler(const int sockfd, queue_t & queue, bool & shouldTerminate)
	: _queue(queue)
	, _sockfd(sockfd)
	, _shouldTerminate(shouldTerminate)
{
	fcntl(_sockfd, F_SETFL, O_NONBLOCK);
}

RadianceRequestHandler::~RadianceRequestHandler()
{
	close(_sockfd);
}

class RadianceRequestHandlerInterruptedException
{
public:
	const char * what() const throw ()
	{
		return "A radiance request handler was interrupted.";
	}
};

void RadianceRequestHandler::operator()()
{
	// Handling a radiance request involves three steps:
	//
	//   1. Send a "Lookup coordinates 2D" message specifying an array of points in uv
	//      space for radiance to sample.  This decides which pixels are included in a
	//      frame.
	//
	//   2. Send a "Get frame" message, specifying a delay which radiance should wait
	//      before sending another frame unprompted.  If the delay is 0, it will wait
	//      for the next "Get frame" message to send the next frame.
	//
	//   3. Listen for a "Frame" messages, convert the frames into a pixel pusher
	//      friendly format, and then push it to the queue.
	
	// If, at any time, one of the steps detects that _shouldTerminate is set to true,
	// a RadianceRequestHandlerInterruptedException will be thrown, and control will
	// be given back to this method so it can finish the handler.

	try
	{
		// Step 1
		INFO("Step 1: sendLookupCoordinates2D()");
		sendLookupCoordinates2D();

		// Step 2
		INFO("Step 2: sendLookupCoordinates2D()");
		sendGetFrame(frameTime);

		// Step 3
		INFO("Step 3: getAndPushFrames()");
		getAndPushFrames();
	}
	catch (RadianceRequestHandlerInterruptedException e)
	{ }
}

enum Commands
{
	DESCRIPTION = 0,
	GET_FRAME = 1,
	FRAME = 2,
	LOOKUP_COORDINATES_2D = 3,
	PHYSICAL_COORDINATES_2D = 4,
	GEOMETRY_2D = 5
};

// Adapted from:
//   https://stackoverflow.com/questions/1001307/detecting-endianness-programmatically-in-a-c-program
bool isBigEndian()
{
    union
    {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

uint32_t toLittleEndian(uint32_t x)
{
	if (!isBigEndian())
	{
		uint8_t temp;
		uint8_t * p = (uint8_t *)&x;
		
		// Swap outer bytes
		temp = p[0];
		p[0] = p[3];
		p[3] = temp;
		
		// Swap inner bytes
		temp = p[1];
		p[1] = p[2];
		p[2] = temp;
	}

	return x;
}

RadianceRequestHandler::sendAll(const char * message, const size_t messageSize)
{
	int bytesToWrite = messageSize;
	int bytesWritten = 0;

	while (bytesWritten < bytesToWrite)
	{

		int newBytesWritten = send(_sockfd, message, messageSize, 0);
		while (newBytesWritten == -1)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				// Writing to socket has caused an error other than needing to block temporarily
				ERR("Failed to send to socket %d: %s", _sockfd, strerror(errno));
				exit(1);
			}

			bytesWritten = send(_sockfd, message, sizeof message, 0);
		}

		bytesWritten += newBytesWritten;
	}
}

void RadianceRequestHandler::readAll(char * buffer, const size_t bufferSize)
{
	// Read first four bytes, which indicate the length of the rest of the message
	int bytesRead = 0;
	int bytesToRead = 4;
	while (bytesRead < bytesToRead)
	{
		int newBytesRead = recv(_sockfd, buffer, bytesToRead);
		while (newBytesRead == -1)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				// Reading from socket has caused an error other than needing to block temporarily
				ERR("Failed to send to socket %d: %s", _sockfd, strerror(errno));
				exit(1);
			}

			usleep(5000);

			newBytesRead = recv(_sockfd, buffer, bytesToRead);
		}

		bytesRead += newBytesRead;
	}
	
}

void RadianceRequestHandler::sendLookupCoordinates2D()
{
	// Packet contents:
	//   Length: 4 bytes, MUST BE LITTLE ENDIAN
	//   Command: 1 byte 
	//   Data: 2 * sizeof float * DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT bytes, array of floats
	
	const int numberOfPixels = DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT;
	char * message = new char[5 + numberOfPixels * 2 * sizeof(float)] {'x', 'x', 'x', 'x', LOOKUP_COORDINATES_2D};

	float * uvCoordinates = (float *)(message + 6);

	float widthDelta =  1 / (float) DANCE_FLOOR_WIDTH;
	float heightDelta = 1 / (float) DANCE_FLOOR_HEIGHT;

	float topLeftX = widthDelta / 2;
	float topLeftY = 1 - heightDelta / 2;

	for (int i = 0; i < numberOfPixels; ++i)
	{
		const int i_modulo_2w = i % (2*DANCE_FLOOR_WIDTH);

		const float x = topLeftX + (i_modulo_2w < DANCE_FLOOR_WIDTH ? i_modulo_2w : 2*DANCE_FLOOR_WIDTH - 1 - i_modulo_2w) * widthDelta;
		const float y = topLeftY - (i / DANCE_FLOOR_WIDTH) * heightDelta;

		uvCoordinates[2*i] = x;
		uvCoordinates[2*i + 1] = y;
	}

	uint32_t * lengthLocation = (uint32_t *)message;
	uint32_t * dataLocation = (uint32_t *)(message + 5);

	*lengthLocation = toLittleEndian(sizeof(message) - 4);
	*dataLocation   = toLittleEndian(frameTime * 1000);

	sendAll(message, 5 + numberOfPixels * 2 * sizeof(float));

	delete[] message;
}

void RadianceRequestHandler::sendGetFrame(uint32_t delay)
{
	// Packet contents:
	//   Length: 4 bytes, MUST BE LITTLE ENDIAN
	//   Command: 1 byte 
	//   Data: 4 bytes, MUST BE LITTLE ENDIAN, used to store delay in ms

	char message[] {'x', 'x', 'x', 'x', GET_FRAME, 'x', 'x', 'x', 'x'};

	uint32_t * lengthLocation = (uint32_t *)message;
	uint32_t * dataLocation = (uint32_t *)(message + 5);

	*lengthLocation = toLittleEndian(sizeof message - 4);
	*dataLocation   = toLittleEndian(delay);

	sendAll(message, sizeof message);
}

void RadianceRequestHandler::getAndPushFrames()
{
	while (!_shouldTerminate)
	{
		Frame frame;
		uint8_t * pixelPusherFrame = (uint8_t *)(frame.data() + 5);
		
		readAll(frameBuffer, sizeof frameBuffer);
		uint8_t * radianceFrame = (uint8_t *)(frameBuffer + 5);

		for (int i = 0; i < DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT; ++i)
		{
			const int alpha = radianceFrame[4*i + 3];

			const int r = radianceFrame[4*i];
			const int g = radianceFrame[4*i + 1];
			const int b = radianceFrame[4*i + 2];

			const uint8_t rAlphaAdjusted = r * alpha / 255;
			const uint8_t gAlphaAdjusted = g * alpha / 255;
			const uint8_t bAlphaAdjusted = b * alpha / 255;

			pixelPusherFrame[3*i]     = rAlphaAdjusted;
			pixelPusherFrame[3*i + 1] = gAlphaAdjusted;
			pixelPusherFrame[3*i + 2] = bAlphaAdjusted;
		}

		BlockingCollectionStatus status = _queue.try_add(std::move(Frame), std::chrono::milliseconds(250));
		while (status == BlockingCollectionStatus::TimedOut)
		{
			if (_shouldTerminate)
			{
				throw RadianceRequestHandlerInterruptedException;
			}

			status = _queue.try_add(std::move(Frame), std::chrono::milliseconds(250));
		}
	}
}
