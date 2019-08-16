#include "RequestHandler.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>
#include <array>
#include <sys/socket.h>
#include "misc/SocketUtilities.h"
#include "misc/Log.h"
#include "ddf.h"

using std::array;

RequestHandler::RequestHandler(const int sockfd, Queue<Frame> & queue, const bool & terminate)
	: _queue(queue)
	, _sockfd(sockfd)
	, _terminate(terminate)
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
		sendGetFrame(100);

		//   3. Listen for a "Frame" messages, convert the frames into a pixel pusher
		//      friendly format, and then push it to the queue.
		//
		getAndPushFrames();

		// If, at any time, one of the steps detects that _terminate is set to true,
		// control will be given back to this method so it can finish the handler.
	}
	catch (OperationInterruptedException e)
	{ }
}



/*======================================== Helper functions ========================================*/
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

constexpr int lengthSize = 4;
constexpr int commandSize = 1;
constexpr int radianceHeaderSize = lengthSize + commandSize;

template <int dataSize>
array<char, dataSize + radianceHeaderSize> getRadianceMessageSkeleton(RadianceCommand command)
{
	array<char, radianceHeaderSize + dataSize> message;

	uint32_t * lengthLocation = (uint32_t *)message.data();
	*lengthLocation = SocketUtilities::hostToLittleEndian(1 + dataSize);

	uint8_t * commandLocation = (uint8_t *)(message.data() + 4);
	*commandLocation = (uint8_t)command;

	return message;
}

static void readRadianceMessage(const int sockfd, RadianceCommand * command, void * dataBuffer,
                                const size_t dataBufferLength, const bool & terminate)
{
	// Read length from socket so we know how many bytes to read for rest of message
	uint32_t lengthLittleEndian;
	SocketUtilities::recvAll(sockfd, &lengthLittleEndian, lengthSize, terminate);
	const uint32_t messageLength = SocketUtilities::littleEndianToHost(lengthLittleEndian);

	// Read command from socket
	uint8_t commandHolder;
	SocketUtilities::recvAll(sockfd, &commandHolder, sizeof commandHolder, terminate);
	assert(commandHolder >= DESCRIPTION && commandHolder <= TUV_MAP);
	*command = static_cast<RadianceCommand>(commandHolder);

	// Read data from socket
	assert(dataBufferLength >= messageLength - 1);
	SocketUtilities::recvAll(sockfd, dataBuffer, messageLength - 1, terminate);
}
/*==================================================================================================*/



void RequestHandler::sendLookupCoordinates2D()
{
	constexpr int width = ddf.width();
	constexpr int height = ddf.height();
	auto message = getRadianceMessageSkeleton<2 * width * height * sizeof (float)>(LOOKUP_COORDINATES_2D);

	auto uvCoordinates = (float (*)[height][2])(message.data() + radianceHeaderSize);

	constexpr float widthStride =  1 / (float) ddf.width();
	constexpr float heightStride = 1 / (float) ddf.height();

	constexpr float uvOriginX = widthStride/2;
	constexpr float uvOriginY = heightStride/2;

	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			uvCoordinates[x][y][0] = uvOriginX + widthStride  * x;
			uvCoordinates[x][y][1] = uvOriginY + heightStride * y;
		}
	}

	SocketUtilities::sendAll(_sockfd, message.data(), message.size(), _terminate);
}

void RequestHandler::sendGetFrame(uint32_t delay)
{
	auto message = getRadianceMessageSkeleton<sizeof (uint32_t)>(GET_FRAME);

	uint32_t * delayLocation = (uint32_t *)(message.data() + radianceHeaderSize);
	*delayLocation   = SocketUtilities::hostToLittleEndian(delay);

	SocketUtilities::sendAll(_sockfd, message.data(), message.size(), _terminate);
}

void RequestHandler::getAndPushFrames()
{
	while (!_terminate)
	{
		// TODO Refactor Frame to possibly write the radiance rgba data into a Frame buffer
		// to improve cache locality by avoiding the copy from two distinct memory buffers
		// that could be allocated at very different parts of the heap

		RadianceCommand command;
		readRadianceMessage(_sockfd, &command, _rgbaBuffer, sizeof _rgbaBuffer, _terminate);
		assert(command == FRAME);

		constexpr int width  = ddf.width();
		constexpr int height = ddf.height();

		Frame frame = Frame::createDanceFloorFrame();
		auto pixelPusherFrame = (uint8_t (*)[height][3])frame.data();

		for (int x = 0; x < width; ++x)
		{
			for (int y = 0; y < height; ++y)
			{
				const int r =     _rgbaBuffer[x][y][0];
				const int g =     _rgbaBuffer[x][y][1];
				const int b =     _rgbaBuffer[x][y][2];
				const int alpha = _rgbaBuffer[x][y][3];

				const uint8_t rAlphaAdjusted = r * alpha / 255;
				const uint8_t gAlphaAdjusted = g * alpha / 255;
				const uint8_t bAlphaAdjusted = b * alpha / 255;

				pixelPusherFrame[x][y][0] = rAlphaAdjusted;
				pixelPusherFrame[x][y][1] = gAlphaAdjusted;
				pixelPusherFrame[x][y][2] = bAlphaAdjusted;
			}
		}

		try
		{
			_queue.add(frame, _terminate, true); // Throw when _terminate == true
		}
		catch (OperationInterruptedException e)
		{
			break;
		}
	}
}
