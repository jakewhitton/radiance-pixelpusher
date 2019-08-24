#ifndef TETROMINO_H
#define TETROMINO_H

#include "Types.h"

class Tetromino
{
	CellContent _type;
	int _orientation;

	CellPosition _boundingBoxPos;
public:
	Tetromino(CellContent type);
	Tetromino(const Tetromino & tetromino) = default;
	Tetromino & operator=(const Tetromino & tetromino) = default;

	void translate(TranslationDirection direction);
	void rotate(RotationDirection direction);
	
	bool collidesWithTetromino(const Tetromino & tetromino) const;

	const CellPosition & getBoundingBoxPosition() const;
	void moveBoundingBox(const CellPosition & pos);

	class const_iterator
	{
		const Tetromino * _tetromino;
		int _i;
	public:
		const_iterator(const Tetromino * tetromino, int i);
		const_iterator operator++();
		Cell operator*();
		bool operator!=(const const_iterator & rhs);
	};

	const_iterator begin() const;
	const_iterator end() const;

	// Static factory method
	static Tetromino generateRandomTetromino();
};

#endif
