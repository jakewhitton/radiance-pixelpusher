#ifndef FRAME_H
#define FRAME_H

#include <vector>
#include <cstddef>
#include <cstdint>

class Frame
{
	std::vector<std::byte> _data;

public:
	Frame() = default;
	Frame(const unsigned size);
	Frame(const Frame & frame) = default;
	Frame(Frame && frame)      = default;

	Frame & operator=(const Frame & frame) = default;
	Frame & operator=(Frame && frame)      = default;

	std::byte * data();
	size_t getSize();

	static Frame createDanceFloorFrame();
};

#endif
