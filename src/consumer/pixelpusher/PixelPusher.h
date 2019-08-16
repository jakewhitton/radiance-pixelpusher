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

	constexpr static size_t sequenceSize      = sizeof (uint32_t);
	constexpr static size_t stripIndicesSize  = sizeof (uint8_t) * ddf.numberOfStrips();
	constexpr static size_t pixelDataSize     = sizeof (uint8_t) * 3 * ddf.numberOfPixels();
	constexpr static size_t messageBufferSize = sequenceSize +
	                                            stripIndicesSize +
	                                            pixelDataSize;
	uint8_t _messageBuffer[messageBufferSize];
	std::array<uint8_t (*)[3], ddf.numberOfStrips()> _stripLocations;


	void writeFrameDataToMessageBuffer(Frame & frame);

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
