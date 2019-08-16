#include "Frame.h"
#include "ddf.h"

using std::byte;

Frame::Frame(const unsigned size)
 	: _data(size)
{ }

byte * Frame::data()
{
	return _data.data();
}

size_t Frame::getSize()
{
	return _data.size();
}

Frame Frame::createDanceFloorFrame()
{
	constexpr size_t frameSize = 3 * sizeof (uint8_t) * ddf.numberOfPixels();
	return Frame(frameSize);
}
