#include "Tetris.h"
#include "misc/Log.h"
#include <unistd.h>
#include <algorithm>
#include <chrono>

Tetris::Tetris()
	: _inputEventQueue(inputEventQueueSize)
	, _inputEventScanner(_inputEventQueue)
	, _remainingLockDelay(0) // Cause tetromino fall and render on first tick
	, _playField()
	, _fallingTetromino(I)
	, _nextTetromino(L)
	, _frame(Frame::createDanceFloorFrame())
	, _frameData((uint8_t (*)[ddf.height()][3])_frame.data())
	, _frameQueue(nullptr)
{
	renderStaticElements();
}

void Tetris::produceFrames(Queue<Frame> & frameQueue)
{
	waitForInputDevicesToConnect();
	_inputEventScanner.start();


	_frameQueue = &frameQueue;

	// Main game loop
	while (_running)
	{
		tick();
	}

	_inputEventScanner.stop();

	_frameQueue = nullptr;
}

void Tetris::stop()
{
	_running = false;
}

/*==================================== Game Logic ===================================*/
void Tetris::waitForInputDevicesToConnect()
{
	while (_running)
	{
		if (_inputEventScanner.inputDevicesConnected())
		{
			break;
		}
		
		constexpr unsigned ms = 10;
		usleep(ms * 1000);
	}
}

void Tetris::handleInputEvent(const InputEvent & inputEvent)
{
	switch (inputEvent.type)
	{
	case TRANSLATE:
		translateFallingTetromino(inputEvent.data.translationDirection);
		break;
	case ROTATE:
		rotateFallingTetromino(inputEvent.data.rotationDirection);
		break;
	default:
		ERR("Invalid value for InputEvent::type");
		exit(1);
	}
}

void Tetris::tick()
{
	// If a lock delay period has expired, process a gravitational moving
	// of the tetromino.
	if (_remainingLockDelay <= 0)
	{
		// Move the tetromino down
		translateFallingTetromino(DOWN);
		
		// Reset lock delay timer
		_remainingLockDelay += lockDelay;
	}
	else
	{
		// Poll for input event
		static constexpr int pollingDelay = 10;
		const int delay = std::min(_remainingLockDelay, pollingDelay);
		InputEvent inputEvent;
		const auto begin = std::chrono::system_clock::now();
		bool eventReceived = _inputEventQueue.take(inputEvent, delay);
		const auto end = std::chrono::system_clock::now();

		// Adjust remaining lock delay for the time spent polling
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
		_remainingLockDelay -= duration.count();

		// If no event was detected, finish processing the tick without rendering
		if (!eventReceived)
		{
			return;
		}

		handleInputEvent(inputEvent);
	}

	render();
}

void Tetris::translateFallingTetromino(TranslationDirection direction)
{
	Tetromino copy = _fallingTetromino;
	copy.translate(direction);
	bool collision = _playField.collidesWithLockedPieces(copy);
	
	if (!collision)
	{
		_fallingTetromino = copy;
		_remainingLockDelay = lockDelay;
	}
	else if (direction == DOWN)
	{
		_score += _playField.commitTetromino(_fallingTetromino);
		_fallingTetromino = _nextTetromino;
		_nextTetromino = Tetromino::generateRandomTetromino();

		_remainingLockDelay = lockDelay;
	}
}

void Tetris::rotateFallingTetromino(RotationDirection direction)
{
	Tetromino copy = _fallingTetromino;
	copy.rotate(direction);
	bool collision = _playField.collidesWithLockedPieces(copy);
	
	if (!collision)
	{
		_fallingTetromino = copy;
		_remainingLockDelay = lockDelay;
	}
}
/*===================================================================================*/



/*==================================== Rendering ====================================*/
static constexpr uint8_t cellColors[][3]
{
	{  0, 255, 255}, // I
	{255, 255,   0}, // O
	{128,   0, 128}, // T
	{  0, 255,   0}, // S
	{255,   0,   0}, // Z
	{  0,   0, 255}, // J
	{255, 165,   0}, // L
	{  0,   0,   0}  // EMPTY
};

void Tetris::render()
{
	renderBoard();
	
	renderFallingTetromino();

	renderNextTetromino();
	
	// Add _frame to the queue to be drawn
	try
	{
		_frameQueue->add(_frame, _running, false); // throw when _running == false
	}
	catch (OperationInterruptedException e)
	{
		return;
	}
}

void Tetris::fillPlayFieldCell(const Cell & cell)
{
	static constexpr int playFieldOriginX = 4;
	static constexpr int playFieldOriginY = 0;
	
	const int startingX = playFieldOriginX + 2*cell.pos.x;
	const int startingY = playFieldOriginY + 2*cell.pos.y;

	const uint8_t * color = cellColors[cell.content];

	// One cell in the board takes up a 2x2 square
	//
	// We fill each of them
	for (int x = startingX; x < startingX + 2; ++x)
	{
		for (int y = startingY; y < startingY + 2; ++y)
		{
			for (int i = 0; i < 3; ++i)
			{
				_frameData[x][y][i] = color[i];
			}
		}
	}
}

void Tetris::renderRectangle(CellPosition p1, CellPosition p2, uint8_t r, uint8_t g, uint8_t b)
{
	for (int x = p1.x; x <= p2.x; ++x)
	{
		for (int y = p1.y; y <= p2.y; ++y)
		{
			_frameData[x][y][0] = r;
			_frameData[x][y][1] = g;
			_frameData[x][y][2] = b;
		}
	}
}

void Tetris::renderStaticElements()
{
	const uint8_t greyR = 200;
	const uint8_t greyG = 200;
	const uint8_t greyB = 200;

	renderRectangle({0, 0}, {3, 39}, greyR, greyG, greyB);

	renderRectangle({4, 40}, {23, 47}, greyR, greyG, greyB);
}

void Tetris::renderBoard()
{
	for (const Cell & cell : _playField)
	{
		fillPlayFieldCell(cell);
	}
}

void Tetris::renderFallingTetromino()
{
	for (const Cell & cell : _fallingTetromino)
	{
		if (_playField.isInsidePlayField(cell.pos))
		{
			fillPlayFieldCell(cell);
		}
	}
}

void Tetris::renderNextTetromino()
{
	// TODO: Render the tetromino that will spawn next in the top left corner
}

void Tetris::renderScore()
{
	// TODO: Render falling tetromino piece
}

void Tetris::renderRemainingTime()
{
	// TODO: Render a display that lets players know how much time is left
}
/*===================================================================================*/
