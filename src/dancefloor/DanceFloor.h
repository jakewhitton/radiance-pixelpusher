#ifndef DANCEFLOOR_H
#define DANCEFLOOR_H

#include <array>
#include <functional>
#include <utility>
#include "misc/Log.h"

struct PixelLocation
{
	int x;
	int y;
	int i; // Position along strip

	bool operator==(const PixelLocation & rhs) const
	{
		return x == rhs.x &&
		       y == rhs.y;
	}

	bool operator<(const PixelLocation & rhs) const
	{
		return x < rhs.x;
	}
};

struct PixelInfo
{
	unsigned stripNumber;
	PixelLocation pos;

	bool operator==(const PixelInfo & rhs) const
	{
		return stripNumber == rhs.stripNumber &&
		       pos == rhs.pos;
	}

	bool operator<(const PixelInfo & rhs) const
	{
		return stripNumber < rhs.stripNumber ||
		       pos < rhs.pos;
	}
};

// So that the above two datatypes can be used in associative
// containers (like std::unordered_map)
namespace std
{

template<>
struct hash<PixelLocation>
{
	size_t operator()(const PixelLocation & pixel) const
	{
		return hash<int>()(pixel.x) ^
		       hash<int>()(pixel.y);
	}
};

template<>
struct hash<PixelInfo>
{
	size_t operator()(const PixelInfo & pixel) const
	{
		return hash<PixelLocation>()(pixel.pos) ^
		       hash<unsigned>()(pixel.stripNumber);
	}
};

}

template <typename StripType, int numStrips>
class DanceFloor
{
	const std::array<StripType, numStrips> _strips;

	template <typename Function>
	constexpr void forAllPixels(Function f) const
	{
		for (const auto & strip : _strips)
		{
			for (const auto & pixel : strip)
			{
				f(pixel);
			}
		}
	}

	template <typename Function>
	constexpr std::pair<int, int> getBoundingInterval(Function f) const
	{
		// Default initializing min and max because constexpr functions
		// can't have uninitialized variables
		bool initialized = false;
		int min = 0;
		int max = 0;

		forAllPixels([&](PixelLocation pixel)
		{
			const int val = f(pixel);

			if (!initialized)
			{
				min = val;
				max = val;
				initialized = true;
			}
			else if (val < min)
			{
				min = val;
			}
			else if (val > max)
			{
				max = val;
			}
		});

		return {min, max};
	}

	template <typename Function>
	constexpr int getRange(Function f) const
	{
		const std::pair<int, int> interval = getBoundingInterval(f);

		return interval.second - interval.first + 1;
	}
public:
	class const_iterator
	{
		const DanceFloor * const _danceFloor;
		unsigned _stripNumber;
		typename StripType::const_iterator _stripIterator;
	public:
		constexpr const_iterator(const DanceFloor * const danceFloor, int stripNumber,
		                         typename StripType::const_iterator stripIterator)
			: _danceFloor(danceFloor)
			, _stripNumber(stripNumber)
			, _stripIterator(stripIterator)
		{ }

		constexpr const_iterator operator++()
		{
			++_stripIterator;

			if (!(_stripIterator != _danceFloor->_strips[_stripNumber].end()) &&
			    _stripNumber != _danceFloor->_strips.size() - 1)
			{
				++_stripNumber;
				_stripIterator = _danceFloor->_strips[_stripNumber].begin();
			}

			return *this;
		}

		constexpr PixelInfo operator*() const
		{
			return {_stripNumber, *_stripIterator};
		}

		constexpr bool operator!=(const const_iterator & rhs) const
		{
			return _danceFloor    != rhs._danceFloor  ||
			       _stripNumber   != rhs._stripNumber ||
			       _stripIterator != rhs._stripIterator;
		}
	};

	constexpr DanceFloor(std::array<StripType, numStrips> strips)
		: _strips(strips)
	{ }

	constexpr const_iterator begin() const
	{
		return {this, 0, _strips.front().begin()};
	}

	constexpr const_iterator end() const
	{
		return {this, numStrips - 1, _strips.back().end()};
	}

	constexpr int numberOfStrips() const
	{
		return numStrips;
	}

	constexpr int numberOfPixels() const
	{
		int sum = 0;
		
		forAllPixels([&](PixelLocation pixel)
		{
			++sum;
		});

		return sum;
	}

	constexpr PixelLocation getOrigin() const
	{
		std::pair<int, int> horizontalInterval = getBoundingInterval([](PixelLocation pixel)
		{
			return pixel.x;
		});

		std::pair<int, int> verticalInterval = getBoundingInterval([](PixelLocation pixel)
		{
			return pixel.y;
		});

		return {horizontalInterval.first, verticalInterval.first};
	}

	constexpr int width() const
	{
		return getRange([](PixelLocation pixel)
		{
			return pixel.x;
		});
	}

	constexpr int height() const
	{
		return getRange([](PixelLocation pixel)
		{
			return pixel.y;
		});
	}
};

#endif
