#include "Frame.h"
#include <arpa/inet.h>
#include <cstring>

using std::array;

uint32_t Frame::seq = 0;

Frame::Frame()
	: _data(new uint8_t[SIZE])
{
	((uint32_t *)_data)[0] = htons(seq++);
}

Frame::Frame(const Frame & frame)
	: Frame()
{
	memcpy(_data, frame._data, SIZE);
}

Frame::Frame(Frame && frame)
	: _data(frame._data)
{
	frame._data = nullptr;
}

Frame & Frame::operator=(const Frame & frame)
{
	memcpy(_data, frame._data, SIZE);
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

const void * Frame::data()
{
	return (const void *)_data;
}

Frame Frame::createFrame(uint8_t r, uint8_t g, uint8_t b)
{
	Frame frame;
	
	uint8_t * data = (uint8_t *)frame.data();
	uint8_t * end = data + Frame::SIZE;

	// Advance past seq
	data += 4;

	// Write strip number and advance
	*data = 1;
	data++;

	// Initialize data
	for (; data < end; data += 3)
	{
		data[0] = r;
		data[1] = g;
		data[2] = b;
	}

	return frame;
}
