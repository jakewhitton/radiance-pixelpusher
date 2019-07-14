#ifndef FRAME_H
#define FRAME_H

#include <vector>
#include <cstddef>
#include <cstdint>
#include "ddf.h"

class Frame
{
	static uint32_t seq;

	size_t _size;
	std::vector<std::byte> _data;

public:
	Frame() = default;
	Frame(const unsigned size);
	Frame(const Frame & frame) = default;
	Frame(Frame && frame)      = default;

	Frame & operator=(const Frame & frame) = default;
	Frame & operator=(Frame && frame)      = default;

	size_t getSize();
	void   setSize(const size_t newSize);

	std::byte * data();

	static Frame createDanceFloorFrame();
};

#endif
