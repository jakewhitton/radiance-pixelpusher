#ifndef PLAYFIELD_H
#define PLAYFIELD_H

#include "Tetromino.h"

class PlayField
{
	static constexpr int playFieldWidth = 10;
	static constexpr int playFieldHeight = 20;
	CellContent _playField[playFieldWidth][playFieldHeight + 2]; // Extra 2 for vanish zone
public:
	PlayField();

	bool collidesWithLockedPieces(const Tetromino & tetromino) const;
	int commitTetromino(const Tetromino & tetromino);
	bool lockedCellsInVanishZone() const;
	bool isInsidePlayField(CellPosition pos) const;

	class const_iterator
	{
		const PlayField * _playField;
		CellPosition _pos;
	public:
		const_iterator(const PlayField * playField, CellPosition pos);
		const_iterator operator++();
		Cell operator*() const;
		bool operator!=(const const_iterator & rhs) const;
	};

	const_iterator begin() const;
	const_iterator end() const;
};

#endif
