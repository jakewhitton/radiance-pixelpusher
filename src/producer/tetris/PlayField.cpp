#include "PlayField.h"
#include "misc/Log.h"
#include <vector>

using std::vector;

PlayField::PlayField()
{
	for (int x = 0; x < playFieldWidth; ++x)
	{
		for (int y = 0; y < playFieldHeight + 2; ++y)
		{
			_playField[x][y] = EMPTY;
		}
	}
}

bool PlayField::collidesWithLockedPieces(const Tetromino & tetromino) const
{
	for (const Cell & cell : tetromino)
	{
		const CellPosition pos = cell.pos;

		// Test for collision
		if (_playField[pos.x][pos.y] != EMPTY)
		{
			return true;
		}

		// Test for out of bounds
		if (pos.x < 0 || pos.x >= playFieldWidth ||
		    pos.y < 0 || pos.y >= playFieldHeight + 2)
		{
			return true;
		}

	}

	return false;
}

int PlayField::commitTetromino(const Tetromino & tetromino)
{
	for (const Cell & cell : tetromino)
	{
		_playField[cell.pos.x][cell.pos.y] = cell.content;
	}

	vector<int> rowsCleared;

	for (int y = 0; y < playFieldHeight; ++y)
	{
		bool hasEmptyCell = false;
		for (int x = 0; x < playFieldWidth; ++x)
		{
			CellContent content = _playField[x][y];

			if (content == EMPTY)
			{
				hasEmptyCell = true;
				break;
			}
		}

		if (!hasEmptyCell)
		{
			rowsCleared.push_back(y);
		}
	}


	// Clear the cleared rows
	for (int row : rowsCleared)
	{
		// Move everything above this row down one
		for (int y = row + 1; y < playFieldHeight + 2; ++y)
		{
			for (int x = 0; x < playFieldWidth; ++x)
			{
				_playField[x][y - 1] = _playField[x][y];
			}
		}
	}

	switch (rowsCleared.size())
	{
	case 0:
		return 0;
	case 1:
		return 25;
	case 2:
		return 50;
	case 3:
		return 100;
	case 4:
		return 200;
	default:
		ERR("Invalid number of rows cleared");
		exit(1);
	}
	return 0;
}

bool PlayField::lockedCellsInVanishZone() const
{
	// TODO: Modify board to include this new tetromino
	return false;
}

bool PlayField::isInsidePlayField(CellPosition pos) const
{
	return pos.x >= 0 && pos.x < playFieldWidth &&
	       pos.y >= 0 && pos.y < playFieldHeight;
}



/*=============================== Iterator interface ================================*/
using const_iterator = PlayField::const_iterator;

const_iterator::const_iterator(const PlayField * playField, CellPosition pos)
	: _playField(playField)
	, _pos(pos)
{ }

const_iterator const_iterator::operator++()
{
	++_pos.x;

	if (_pos.x == playFieldWidth)
	{
		_pos.x = 0;
		++_pos.y;
	}

	return *this;
}

Cell const_iterator::operator*() const
{
	return {_pos, _playField->_playField[_pos.x][_pos.y]};
}

bool const_iterator::operator!=(const const_iterator & rhs) const
{
	return _playField != rhs._playField ||
	       _pos       != rhs._pos;
}

const_iterator PlayField::begin() const
{
	return {this, {0, 0}};
}

const_iterator PlayField::end() const
{
	return {this, {0, playFieldHeight}};
}
/*===================================================================================*/
