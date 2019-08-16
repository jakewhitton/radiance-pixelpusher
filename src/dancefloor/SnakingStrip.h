#ifndef SNAKINGSTRIP
#define SNAKINGSTRIP

#include <cstdlib>
#include <stdexcept>

enum SnakeType
{
	SNAKE_VERTICAL,
	SNAKE_HORIZONTAL
};

class SnakingStrip
{
	const PixelLocation _start;

	struct Vec2D
	{
		int x, y;
	};

	const int _parallelDistance;
	const int _orthogonalDistance;

	const Vec2D _parallelUnitVector;
	const Vec2D _orthogonalUnitVector;
	
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

		size_t operator-(const const_iterator & rhs)
		{
			return _i - rhs._i;
		}

                constexpr PixelLocation operator*() const
		{
			// Normalize i to [0, 2 * _parallelDistance), as the parallel
			// coordinate is periodic in that interval
			const int n = _i % (2 * _strip->_parallelDistance);
			const int parallelCoord = n < _strip->_parallelDistance ?
			                          n
				                      :
						              2*_strip->_parallelDistance - 1 - n;
			
			const int orthogonalCoord  = _i / _strip->_parallelDistance;

			const int x = _strip->_start.x +
				          parallelCoord * _strip->_parallelUnitVector.x +
				          orthogonalCoord * _strip->_orthogonalUnitVector.x;

			const int y = _strip->_start.y +
				          parallelCoord * _strip->_parallelUnitVector.y +
				          orthogonalCoord * _strip->_orthogonalUnitVector.y;

			return {x, y, _i};
		}

                constexpr bool operator!=(const const_iterator & rhs) const
		{
			return _i     != rhs._i   ||
			       _strip != rhs._strip;
		}
	};

	constexpr SnakingStrip(const PixelLocation start, const int deltaX, const int deltaY, const SnakeType type)
		: _start(start)
		, _parallelDistance    (type == SNAKE_HORIZONTAL ?
							    abs(deltaX)
								:
								abs(deltaY))
		, _orthogonalDistance  (type == SNAKE_HORIZONTAL ?
								abs(deltaY)
								:
								abs(deltaX))		  
		, _parallelUnitVector  (type == SNAKE_HORIZONTAL ?
		                        Vec2D {deltaX / _parallelDistance, 0}
								:
		                        Vec2D {0, deltaY / _parallelDistance})
		, _orthogonalUnitVector(type == SNAKE_HORIZONTAL ?
		                        Vec2D {0, deltaY / _orthogonalDistance}
		                        :
	                            Vec2D {deltaX / _orthogonalDistance})
	{ }

	constexpr const_iterator begin() const
	{
		return {this, 0};
	}

	constexpr const_iterator end() const
	{
		return {this, _parallelDistance * _orthogonalDistance};
	}
};

#endif
