#include "Frame.h"
#include "ddf.h"

using std::byte;

uint32_t Frame::seq = 0;

Frame::Frame(const unsigned size)
	: _size(size)
	, _data(size)
{
	uint32_t * seqPointer = (uint32_t *)_data.data();
	seqPointer[0] = seq++;
}

size_t Frame::getSize()
{
	return _size;
}

void Frame::setSize(const size_t newSize)
{
	if (newSize > _size)
	{
		_data.resize(newSize);
	}

	_size = newSize;
}

byte * Frame::data()
{
	return _data.data();
}

Frame Frame::createDanceFloorFrame()
{
	constexpr size_t sequenceNumberSize = sizeof (uint32_t);
	constexpr size_t stripIndexDataSize = sizeof (uint8_t) * ddf.numberOfStrips();
	constexpr size_t colorDataSize      = 3 * sizeof (uint8_t) * ddf.numberOfPixels();

	constexpr size_t frameSize = sequenceNumberSize + stripIndexDataSize + colorDataSize;

	return Frame(frameSize);
}
