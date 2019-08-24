#ifndef TYPES_H
#define TYPES_H

enum TranslationDirection
{
	LEFT,
	DOWN,
	RIGHT
};

enum RotationDirection
{
	CLOCKWISE,
	COUNTER_CLOCKWISE
};

struct CellPosition
{
	int x, y;

	bool operator==(const CellPosition & rhs) const
	{
		return x == rhs.x &&
		       y == rhs.y;
	}

	bool operator!=(const CellPosition & rhs) const
	{
		return x != rhs.x ||
		       y != rhs.y;
	}
};

enum CellContent
{
	I = 0,
	O,
	T,
	S,
	Z,
	J,
	L,
	EMPTY
};

struct Cell
{
	CellPosition pos;
	CellContent content;
};

#endif
