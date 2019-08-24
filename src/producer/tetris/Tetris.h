#ifndef TETRIS_H
#define TETRIS_H

#include "producer/FrameProducer.h"
#include "InputEventScanner.h"
#include "Tetromino.h"
#include "PlayField.h"
#include "ddf.h"

class Tetris
	: public FrameProducer
{
	bool _running = true;

	// Input event handling
	static constexpr unsigned inputEventQueueSize = 10;
	Queue<InputEvent> _inputEventQueue;
	InputEventScanner _inputEventScanner;
	void waitForInputDevicesToConnect();
	void handleInputEvent(const InputEvent & inputEvent);

	// Lock delay handling
	static constexpr int lockDelay = 100;
	int _remainingLockDelay;

	// Play field
	PlayField _playField;
	
	// Falling tetromino
	Tetromino _fallingTetromino;
	
	// Queued tetromino (the next to spawn)
	Tetromino _nextTetromino;

	// Game logic
	void tick();
	void translateFallingTetromino(TranslationDirection direction);
	void rotateFallingTetromino(RotationDirection direction);

	// Rendering
	Frame _frame;
	uint8_t (*_frameData)[ddf.height()][3];
	Queue<Frame> * _frameQueue;
	void render();
	void fillPlayFieldCell(const Cell & cell);
	void renderBoard();
	void renderFallingTetromino();
	void renderNextTetromino();
	void renderScore();
	void renderRemainingTime();
public:
	Tetris();
	void produceFrames(Queue<Frame> & frameQueue) override;
	void stop() override;
};

#endif
