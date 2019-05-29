#include "RadianceRequestHandler.h"
#include "config.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "misc/Log.h"
#include "misc/SocketUtilities.h"

using code_machina::BlockingCollectionStatus;

RadianceRequestHandler::RadianceRequestHandler(const int sockfd, FrameQueue & queue, bool & shouldTerminate)
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
	try
	{
		// Handling a radiance request involves three steps:
		//
		//   1. Send a "Lookup coordinates 2D" message specifying an array of points in uv
		//      space for radiance to sample.  This decides which pixels are included in a
		//      frame.
                //
		sendLookupCoordinates2D();

		//   2. Send a "Get frame" message, specifying a delay which radiance should wait
		//      before sending another frame unprompted.  If the delay is 0, it will wait
		//      for the next "Get frame" message to send the next frame.
		//
		sendGetFrame(frameTime);

		//   3. Listen for a "Frame" messages, convert the frames into a pixel pusher
		//      friendly format, and then push it to the queue.
		//
		getAndPushFrames();

		// If, at any time, one of the steps detects that _shouldTerminate is set to true,
		// a RadianceRequestHandlerInterruptedException will be thrown, and control will
		// be given back to this method so it can finish the handler.
	}
	catch (RadianceRequestHandlerInterruptedException e)
	{ }
	catch (SocketIOInterruptedException e)
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

void RadianceRequestHandler::sendLookupCoordinates2D()
{
	// Packet contents:
	//   Length: 4 bytes, MUST BE LITTLE ENDIAN
	//   Command: 1 byte 
	//   Data: 2 * sizeof float * DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT bytes, array of floats
	
	const int numberOfPixels = DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT;

	char * message = new char[5 + numberOfPixels * 2 * sizeof(float)] {'x', 'x', 'x', 'x', LOOKUP_COORDINATES_2D};


	uint32_t * lengthLocation = (uint32_t *)message;
	*lengthLocation = SocketUtilities::hostToLittleEndian(1 + numberOfPixels * 2 * sizeof(float));

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

	SocketUtilities::sendAll(_sockfd, message, 5 + numberOfPixels * 2 * sizeof(float), _shouldTerminate);

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

	*lengthLocation = SocketUtilities::hostToLittleEndian(sizeof message - 4);
	*dataLocation   = SocketUtilities::hostToLittleEndian(delay);

	SocketUtilities::sendAll(_sockfd, message, sizeof message, _shouldTerminate);
}

void RadianceRequestHandler::getAndPushFrames()
{
	while (!_shouldTerminate)
	{
		Frame frame;
		uint8_t * pixelPusherFrame = (uint8_t *)(frame.data() + 5);
		
		SocketUtilities::recvAll(_sockfd, frameBuffer, sizeof frameBuffer, _shouldTerminate);

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

		bool added = _queue.add(frame, std::chrono::milliseconds(100));
		while (!added)
		{
			if (_shouldTerminate)
			{
				throw RadianceRequestHandlerInterruptedException();
			}

			added = _queue.add(frame, std::chrono::milliseconds(100));
		}
	}
}
