#ifndef PIXELPUSHER_H
#define PIXELPUSHER_H

#include "consumer/FrameConsumer.h"
#include <array>
#include "ddf.h"

class PixelPusher
	: public FrameConsumer
{
private:
	const int _sockfd;
	bool _running;

	constexpr static int stripsInPacket       = 1;
	constexpr static int pixelsInStrip        = 12 * 16;
	constexpr static size_t sequenceSize      = sizeof (uint32_t);
	constexpr static size_t stripIndicesSize  = sizeof (uint8_t) * stripsInPacket;
	constexpr static size_t pixelDataSize     = sizeof (uint8_t) * 3 * pixelsInStrip * stripsInPacket;
	constexpr static size_t messageBufferSize = sequenceSize +
	                                            stripIndicesSize +
	                                            pixelDataSize;
	uint8_t _messageBuffer[messageBufferSize];

	void writeFrameDataToMessageBuffer(Frame & frame, int lowStrip, int highStrip);

public:
	// Don't allow copying or moving
	PixelPusher(const PixelPusher &) = delete;
	PixelPusher(PixelPusher &&)      = delete;

	PixelPusher(const char * location, const char * port);
	~PixelPusher();

	void consumeFrames(Queue<Frame> & frameQueue) override;
	void stop() override;
};

#endif
