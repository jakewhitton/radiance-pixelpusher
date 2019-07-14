#ifndef SNAKINGSTRIP
#define SNAKINGSTRIP

enum SnakeType
{
	SNAKE_VERTICAL,
	SNAKE_HORIZONTAL
};

class SnakingStrip
{
	const PixelLocation _start;
	const int _length;
	const int _width;
	const SnakeType _type;
public:
	class const_iterator
	{
		const SnakingStrip * _strip;
		int _i;
	public:
		constexpr const_iterator(const SnakingStrip * strip, int i)
			: _strip(strip)
			, _i(i)
		{ }

		constexpr const_iterator operator++()
		{
			++_i;

			return *this;
		}

                constexpr PixelLocation operator*() const
		{
			// TODO make this code work for negative widths and heights
			
			const int iMod2L = _i % (2*_strip->_length);
			const int lengthOffset = iMod2L < _strip->_length ?
			                         iMod2L
						 :
						 2*_strip->_length - 1 - iMod2L;
			
			const int widthOffset  = _i / _strip->_length;

			switch (_strip->_type)
			{
			case SNAKE_HORIZONTAL:
				return {_strip->_start.x + lengthOffset,
				        _strip->_start.y + widthOffset};
			case SNAKE_VERTICAL:
				return {_strip->_start.x + widthOffset,
				        _strip->_start.y + lengthOffset};
			default:
				ERR("Unexpected value for SnakingStrip::_type");
				exit(1);
			}
		}

                constexpr bool operator!=(const const_iterator & rhs) const
		{
			return _i     != rhs._i   ||
			       _strip != rhs._strip;
		}
	};

	// length - the distance from the start to the first point where the snake changes direction
	// width - the horizontal distance the snake must make before reaching the end
	//
	// e.g.
	//
	// ->->->->|
	// <-<-<-<-|
	//
	// length = 9
	// width = 2
	constexpr SnakingStrip(const PixelLocation start, const int length, const int width, const SnakeType type)
		: _start(start)
		, _length(length)
		, _width(width)
		, _type(type)
	{ }

	constexpr const_iterator begin() const
	{
		return {this, 0};
	}

	constexpr const_iterator end() const
	{
		return {this, _length * _width};
	}
};

#endif
