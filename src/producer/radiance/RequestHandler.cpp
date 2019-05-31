#include "RequestHandler.h"
#include "config.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>
#include <sys/socket.h>
#include "misc/SocketUtilities.h"
#include "misc/Log.h"

RequestHandler::RequestHandler(const int sockfd, FrameQueue & queue,
                                               const bool & shouldTerminate)
	: _queue(queue)
	, _sockfd(sockfd)
	, _shouldTerminate(shouldTerminate)
{
	fcntl(_sockfd, F_SETFL, O_NONBLOCK);
}

RequestHandler::~RequestHandler()
{
	close(_sockfd);
}

void RequestHandler::operator()()
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
		// control will be given back to this method so it can finish the handler.
	}
	catch (OperationInterruptedException e)
	{ }
}

enum RadianceCommand
{
	DESCRIPTION              = 0, // Get info about Radiance client
	GET_FRAME                = 1, // Ask for a frame, MUST ISSUE LOOKUP_COORDINATES_2D FIRST
	FRAME                    = 2, // Command for a frame message from Radiance
	LOOKUP_COORDINATES_2D    = 3, // Give uv coordinates that identify pixel locations
	PHYSICAL_COORDINATES_2D  = 4, // Give uv coordinates for radiance to visualize pixel locations
	GEOMETRY_2D              = 5, // Give a png image to visualize pixels against (to see alpha)
	LOOKUP_COORDINATES_3D    = 6, // Not yet supported by Radiance
	PHYSICAL_COORDINATES_3D  = 7, // Not yet supported by Radiance
	GEOMETRY_3D              = 8, // Not yet supported by Radiance
	TUV_MAP                  = 9  // Not yet supported by Radiance
};

void RequestHandler::sendLookupCoordinates2D()
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

void RequestHandler::sendGetFrame(uint32_t delay)
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

static void readRadianceMessage(const int sockfd, RadianceCommand * command, void * dataBuffer,
                                const size_t dataBufferLength, const bool & terminate);

void RequestHandler::getAndPushFrames()
{
	while (!_shouldTerminate)
	{
		RadianceCommand command;
		readRadianceMessage(_sockfd, &command, rgbaBuffer, sizeof rgbaBuffer, _shouldTerminate);
		assert(command == FRAME);

		Frame frame;
		uint8_t * pixelPusherFrame = (uint8_t *)(frame.data() + 5);
		
		for (int i = 0; i < DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT; ++i)
		{
			const int alpha = rgbaBuffer[4*i + 3];

			const int r = rgbaBuffer[4*i];
			const int g = rgbaBuffer[4*i + 1];
			const int b = rgbaBuffer[4*i + 2];

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
				return;
			}

			added = _queue.add(frame, std::chrono::milliseconds(100));
		}
	}
}

/*=================================== Helper functions ===================================*/
static void readRadianceMessage(const int sockfd, RadianceCommand * command, void * dataBuffer,
                                const size_t dataBufferLength, const bool & terminate)
{
	/*
	 * Radiance messages have a specific format, which can be found at:
	 *
	 * https://github.com/zbanks/radiance/blob/master/light_output.md
	 *
	 * The format for a message is as follows:
	 *
	 * +---------------------------------------------------------------+
	 * | Length (4 bytes) | Command (1 byte) | Data (Length - 1 bytes) |
	 * +---------------------------------------------------------------+
	 *
	 * Note that length (and any other integral values longer than one byte) are assumed to
	 * be little endian unless otherwise specified, so you must convert them to host endianness.
	 */

	// Read length from socket so we know how many bytes to read for rest of message
	uint32_t lengthLittleEndian;
	SocketUtilities::recvAll(sockfd, &lengthLittleEndian, sizeof lengthLittleEndian, terminate);
	const uint32_t messageLength = SocketUtilities::littleEndianToHost(lengthLittleEndian);

	// Read command from socket
	uint8_t commandHolder;
	SocketUtilities::recvAll(sockfd, &commandHolder, sizeof commandHolder, terminate);
	assert(commandHolder < 10);
	*command = static_cast<RadianceCommand>(commandHolder);

	// Read data from socket
	assert(dataBufferLength >= messageLength - 1);
	SocketUtilities::recvAll(sockfd, dataBuffer, messageLength - 1, terminate);
}
/*========================================================================================*/
