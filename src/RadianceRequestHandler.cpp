#include "RadianceRequestHandler.h"
#include "config.h"
#include <cstdlib>
#include <unistd.h>
#include "Log.h"

RadianceRequestHandler::RadianceRequestHandler(const int sockfd, queue_t & queue, bool & shouldTerminate)
	: _queue(queue)
	, _sockfd(sockfd)
	, _shouldTerminate(shouldTerminate)
{ }

RadianceRequestHandler::~RadianceRequestHandler()
{
	close(_sockfd);
}

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


	INFO("In the boyo with socket %d", _sockfd);

	try
	{
		// Step 1
		sendLookupCoordinates2D();

		// Step 2
		sendGetFrame(frameTime);

		// Step 3
		getAndPushFrames();
	}
	catch (RadianceRequestHandlerInterruptedException e)
	{ }
}

static enum Commands
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
		uint8_t * p = &x;
		
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

static void sendAll(const int sockfd, const char * message, const size_t messageSize)
{
	int bytesToWrite = messageSize;
	int bytesWritten = 0;

	while (bytesWritten < bytesToWrite)
	{

		
		int newBytesWritten = send(_sockfd, message, messageSize, 0);
		while (bytesWritten == -1)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				// Writing to socket has caused an error other than needing to block temporarily
				ERR("Failed to send \"lookup coordinates 2D\" to radiance: %s", strerror());
				exit(1);
			}

			bytesWritten = send(_sockfd, message, sizeof message, 0);
		}

		bytesWritten += newBytesWritten;
	}
}

void RadianceRequestHandler::sendLookupCoordinates2D()
{
	// Packet contents:
	//   Length: 4 bytes, MUST BE LITTLE ENDIAN
	//   Command: 1 byte 
	//   Data: 2 * sizeof float * DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT bytes, array of floats
	
	const int numberOfPixels = DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT;
	char * message = new char[5 + numberOfPixels * 2 * sizeof float] {'x', 'x', 'x', 'x', LOOKUP_COORDINATES_2D};

	float * uvCoordinates = (float *)(message + 6);

	float widthDelta =  1 / (float) DANCE_FLOOR_WIDTH;
	float heightDelta = 1 / (float) DANCE_FLOOR_HEIGHT;

	float topLeftX = widthDelta / 2;
	float topLefty = 1 - hightDelta / 2;

	for (int i = 0; i < numberOfPixels; ++i)
	{
		int x_i = i % DANCE_FLOOR_WIDTH

		const float x = topLeftX + (DANCE_FLOOR_WIDTH - abs(x_i - DANCE_FLOOR_WIDTH)) * widthDelta;
		const float y = topLeftY;

		uvCoordinates[2*i] = x;
		uvCoordinates[2*i + 1] = y;
	}

	uint32_t * lengthLocation = message;
	uint32_t * dataLocation = (uint32_t *)(message + 5);

	*lengthLocation = toLittleEndian(sizeof message - 4);
	*dataLocation   = toLittleEndian(frameTime * 1000);

	sendAll(_sockfd, message, 5 + numberOfPixels * 2 * sizeof float);

	delete[] message;
}

void RadianceRequestHandler::sendGetFrame(uint32_t delay)
{
	// Packet contents:
	//   Length: 4 bytes, MUST BE LITTLE ENDIAN
	//   Command: 1 byte 
	//   Data: 4 bytes, MUST BE LITTLE ENDIAN, used to store delay in ms

	char message[] {'x', 'x', 'x', 'x', GET_FRAME, 'x', 'x', 'x', 'x'};

	uint32_t * lengthLocation = message;
	uint32_t * dataLocation = (uint32_t *)(message + 5);

	*lengthLocation = toLittleEndian(sizeof message - 4);
	*dataLocation   = toLittleEndian(delay);

	sendAll(_sockfd, message, sizeof message);
}

void RadianceRequestHandler::getAndPushFrames()
{

}
