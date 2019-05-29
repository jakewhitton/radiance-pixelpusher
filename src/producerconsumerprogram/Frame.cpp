#include "Frame.h"
#include <arpa/inet.h>
#include <cstring>
#include <array>
#include <random>
#include "misc/Log.h"

using std::array;

uint32_t Frame::seq = 0;

Frame::Frame()
	: _data(new uint8_t[SIZE])
{
	uint32_t * seqPointer = (uint32_t *)_data;
	seqPointer[0] = seq;
	++seq;
}

Frame::Frame(const Frame & frame)
	: _data(new uint8_t[SIZE])
{
	memcpy(_data, frame._data, SIZE);
	uint32_t * seqPointer = (uint32_t *)_data;
	seqPointer[0] = ((uint32_t *)frame._data)[0];
}

Frame::Frame(Frame && frame)
	: _data(frame._data)
{
	frame._data = nullptr;
}

Frame & Frame::operator=(const Frame & frame)
{
	memcpy(_data, frame._data, SIZE);
	uint32_t * seqPointer = (uint32_t *)_data;
	seqPointer[0] = ((uint32_t *)frame._data)[0];

	return *this;
}

Frame & Frame::operator=(Frame && frame)
{
	_data = frame._data;
	frame._data = nullptr;
	return *this;
}

Frame::~Frame()
{
	delete[] _data;
}

size_t size()
{
	return sizeof (uint8_t *);
}

const char * Frame::data()
{
	return (const char *)_data;
}

array<uint8_t, DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT * 3> createPixelData()
{
	array<uint8_t, DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT * 3> data;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> random(0, 255);

	for (uint8_t & color : data)
	{
		color = random(gen);
	}

	return data;
}

Frame Frame::createRainbowFrame()
{
	static unsigned shift = 0;

	static array<uint8_t, DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT * 3> pixelData = createPixelData();

	Frame frame;
	
	uint8_t * data = ((uint8_t *)frame.data()) + 5; // 4 for seq, 1 for index

	int i = 0;

	// Write strip number and advance
	data[i - 1] = 0;

	// Initialize data
	for (; i < DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT; ++i)
	{
		uint8_t * pixel = pixelData.data() + 3 * ((i + shift) % DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT);
		data[3*i]     = pixel[0];
		data[3*i + 1] = pixel[1];
		data[3*i + 2] = pixel[2];
	}

	++shift;

	return frame;
}

Frame Frame::createFrame(uint8_t r, uint8_t g, uint8_t b)
{
	Frame frame;
	
	uint8_t * data = ((uint8_t *)frame.data()) + 5; // 4 for seq, 1 for index

	int i = 0;

	// Write strip number and advance
	data[i - 1] = 0;

	// Initialize data
	for (; i < DANCE_FLOOR_WIDTH * DANCE_FLOOR_HEIGHT; ++i)
	{
		data[3*i]     = r;
		data[3*i + 1] = g;
		data[3*i + 2] = b;
	}

	return frame;
}
