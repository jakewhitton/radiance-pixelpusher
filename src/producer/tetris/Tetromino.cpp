#include "Tetromino.h"
#include "misc/Log.h"
#include <cassert>

Tetromino::Tetromino(CellContent type)
	: _type(type)
	, _orientation(0)
	, _boundingBoxPos({0, 19})
{
	// You cannot construct a tetromino from the EMPTY cell content
	assert(type != EMPTY);
}

void Tetromino::translate(TranslationDirection direction)
{
	switch (direction)
	{
	case LEFT:
		--_boundingBoxPos.x;
		break;
	case DOWN:
		--_boundingBoxPos.y;
		break;
	case RIGHT:
		++_boundingBoxPos.x;
		break;
	default:
		ERR("Invalid value for tetromino translation direction");
	}
}

void Tetromino::rotate(RotationDirection direction)
{
	switch (direction)
	{
	case CLOCKWISE:
		_orientation = (_orientation + 1) % 4;
		break;
	case COUNTER_CLOCKWISE:
		_orientation = (_orientation + 3) % 4;
		break;
	default:
		ERR("Invalid value for tetromino rotation direction");
	}
}

bool Tetromino::collidesWithTetromino(const Tetromino & tetromino) const
{
	// TODO: detect collision between tetrominos
	return false;
}

const CellPosition & Tetromino::getBoundingBoxPosition() const
{
	return _boundingBoxPos;
}

void Tetromino::moveBoundingBox(const CellPosition & pos)
{
	_boundingBoxPos = pos;
}


/*=============================== Iterator Interface ================================*/
using const_iterator = Tetromino::const_iterator;


const_iterator::const_iterator(const Tetromino * tetromino, int i)
	: _tetromino(tetromino)
	, _i(i)
{ }

const_iterator const_iterator::operator++()
{
	++_i;

	return *this;
}

// Tetromino shape data
static constexpr CellPosition shapeData[7][4][4]
{
	// I
	{
		{{0, 2}, {1, 2}, {2, 2}, {3, 2}},
		{{2, 3}, {2, 2}, {2, 1}, {2, 0}},
		{{3, 1}, {2, 1}, {1, 1}, {0, 1}},
		{{1, 0}, {1, 1}, {1, 2}, {1, 3}}
	},

	// O
	{
		{{1, 1}, {2, 1}, {2, 2}, {1, 2}},
		{{1, 1}, {2, 1}, {2, 2}, {1, 2}},
		{{1, 1}, {2, 1}, {2, 2}, {1, 2}},
		{{1, 1}, {2, 1}, {2, 2}, {1, 2}}
	},

	// T
	{
		{{0, 1}, {1, 1}, {2, 1}, {1, 2}},
		{{1, 2}, {1, 1}, {1, 0}, {2, 1}},
		{{2, 1}, {1, 1}, {0, 1}, {1, 0}},
		{{1, 0}, {1, 1}, {1, 2}, {0, 1}}
	},

	// S
	{
		{{0, 1}, {1, 1}, {1, 2}, {2, 2}},
		{{1, 2}, {1, 1}, {2, 1}, {2, 0}},
		{{2, 1}, {1, 1}, {1, 0}, {0, 0}},
		{{1, 0}, {1, 1}, {0, 1}, {0, 2}}
	},

	// Z
	{
		{{0, 2}, {1, 2}, {1, 1}, {2, 1}},
		{{2, 2}, {2, 1}, {1, 1}, {1, 0}},
		{{2, 0}, {1, 0}, {1, 1}, {0, 1}},
		{{0, 0}, {0, 1}, {1, 1}, {1, 2}}
	},

	// J
	{
		{{0, 2}, {0, 1}, {1, 1}, {2, 1}},
		{{2, 2}, {1, 2}, {1, 1}, {1, 0}},
		{{2, 0}, {2, 1}, {1, 1}, {0, 1}},
		{{0, 0}, {1, 0}, {1, 1}, {1, 2}}
	},

	// L
	{
		{{0, 1}, {1, 1}, {2, 1}, {2, 2}},
		{{1, 2}, {1, 1}, {1, 0}, {2, 0}},
		{{2, 1}, {1, 1}, {0, 1}, {0, 0}},
		{{1, 0}, {1, 1}, {1, 2}, {0, 2}}
	}
};

Cell const_iterator::operator*()
{
	const CellPosition & pos = shapeData[_tetromino->_type][_tetromino->_orientation][_i];
	const CellPosition & boundingBox = _tetromino->_boundingBoxPos;

	return {{pos.x + boundingBox.x, pos.y + boundingBox.y}, _tetromino->_type};
}

bool const_iterator::operator!=(const const_iterator & rhs)
{
	return _tetromino != rhs._tetromino ||
	       _i         != rhs._i;
}

const_iterator Tetromino::begin() const
{
	return {this, 0};
}

const_iterator Tetromino::end() const
{
	return {this, 4};
}
/*===================================================================================*/

Tetromino Tetromino::generateRandomTetromino()
{
	int r = rand() % 7;
	switch (r) {
		case 0: return Tetromino(I); break;
		case 1: return Tetromino(O); break;
		case 2: return Tetromino(T); break;
		case 3: return Tetromino(S); break;
		case 4: return Tetromino(Z); break;
		case 5: return Tetromino(J); break;
		default: return Tetromino(L); break; }
}
