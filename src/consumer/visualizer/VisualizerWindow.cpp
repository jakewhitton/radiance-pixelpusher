#include "VisualizerWindow.h"
#include "misc/Log.h"

using std::thread;

VisualizerWindow::VisualizerWindow(Queue<DanceFloorUpdate> & updateQueue)
	: _running(true)
	, _updateQueue(updateQueue)
	, _window(nullptr)
	, _renderer(nullptr)
	, _refresh(false)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		SDL_InitSubSystem(SDL_INIT_VIDEO);
	}

}

VisualizerWindow::~VisualizerWindow()
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void VisualizerWindow::createWindow()
{
	const char * title  = "Dance Floor Visualizer";
	const int    xPos   = SDL_WINDOWPOS_CENTERED;
	const int    yPos   = SDL_WINDOWPOS_CENTERED;
	const int    width  = 800;
	const int    height = 600;
	const int    flags  = SDL_WINDOW_SHOWN |
			      SDL_WINDOW_RESIZABLE;
	_window = SDL_CreateWindow(title, xPos, yPos, width, height, flags);

	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

	// Clear the window
	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(_renderer);
}

void VisualizerWindow::processEvents()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
		{
			_running = false;
		}
	}

	if (_refresh && _lastUpdate)
	{
		// Clear the screen
		SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(_renderer);

		// Figure out the value for things like how big cells should be and
		// where they should be put based on the window width and height
		setSpacingParameters();

		// Redraw dance floor
		for (const PixelUpdate & update : *_lastUpdate)
		{
			fillCell(update.x, update.y, update.r, update.g, update.b);
		}

		_refresh = false;
	}
}

void VisualizerWindow::destroyWindow()
{
	if (_window)
	{
		SDL_DestroyWindow(_window);
	}

	if (_renderer)
	{
		SDL_DestroyRenderer(_renderer);
	}
}

void VisualizerWindow::setSpacingParameters()
{
	_squareSize = 25;
	_originX = 5;
	_originY = 5;
}

SDL_Rect VisualizerWindow::getCellRect(int x, int y)
{
	const int xPos   = _originX + _squareSize * x;
	const int yPos   = _originY + _squareSize * y;
	const int width  = _squareSize;
	const int height = _squareSize;
	
	return {xPos, yPos, width, height};
}

void VisualizerWindow::fillCell(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	SDL_Rect cellRect = getCellRect(x, y);

	SDL_SetRenderDrawColor(_renderer, r, g, b, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(_renderer, &cellRect);
}

void VisualizerWindow::run()
{
	createWindow();

	while (_running)
	{
		DanceFloorUpdate update;
		constexpr unsigned frameTime = 1000 / frameRate;
		bool updateReceived = _updateQueue.take(update, frameTime);
		if (updateReceived)
		{
			_lastUpdate = std::move(update);
			_refresh = true;
		}

		processEvents();

		SDL_RenderPresent(_renderer);
	}

	destroyWindow();
}

void VisualizerWindow::stop()
{
	_running = false;
}

