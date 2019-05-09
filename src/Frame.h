#ifndef FRAME_H
#define FRAME_H

#include <array>
#include <cstdint>
#include "BlockingCollection.h"
#include "config.h"


class Frame
{

	static uint32_t seq;

	uint8_t * _data;
public:
	constexpr static const int SIZE = 4                  // Sequence number
	                                + 1                  // Strip index
	                                + (DANCE_FLOOR_WIDTH
	                                * DANCE_FLOOR_HEIGHT // Pixel data
	                                * 3);
	Frame();

	Frame(const Frame & frame);
	Frame(Frame && frame);

	Frame & operator=(const Frame & frame);
	Frame & operator=(Frame && frame);

	~Frame();

	size_t size();

	static Frame createRainbowFrame();
	static Frame createFrame(uint8_t r, uint8_t g, uint8_t b);

	const void * data();
};

#endif
