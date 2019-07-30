#include "VisualizerWindow.h"
#include "misc/Log.h"
#include <algorithm>
#include <SDL2/SDL_ttf.h>

using std::thread;

VisualizerWindow::VisualizerWindow(Queue<DanceFloorUpdate> & updateQueue)
	: _running(true)
	, _updateQueue(updateQueue)
	, _window(nullptr)
	, _renderer(nullptr)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		SDL_InitSubSystem(SDL_INIT_VIDEO);
	}

	if (!TTF_WasInit())
	{
		TTF_Init();
	}
}

VisualizerWindow::~VisualizerWindow()
{
	TTF_Quit();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
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
		}

		// Handle keypresses, window events, etc
		handleEvents();

		// If window size has changed, recalculate sizing parameters
		int windowWidth = 0, windowHeight = 0;
		SDL_GetWindowSize(_window, &windowWidth, &windowHeight);
		if (windowWidth != _windowWidth || windowHeight != _windowHeight)
		{
			_windowWidth = windowWidth;
			_windowHeight = windowHeight;
			setSpacingParameters();
		}

		clearWindow();

		if (_lastUpdate)
		{
			drawDanceFloor();
		}
		else
		{
			drawWaitingMessage();
		}

		SDL_RenderPresent(_renderer);
	}

	destroyWindow();
}

void VisualizerWindow::stop()
{
	_running = false;
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
	SDL_GetWindowSize(_window, &_windowWidth, &_windowHeight);
	setSpacingParameters();

	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
}

void VisualizerWindow::handleEvents()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_QUIT:
		{
			_running = false;
			break;
		}
		case SDL_KEYDOWN:
		{
			switch (e.key.keysym.sym)
			{
			case SDLK_r:
			{
				// TODO Change rotation
				break;
			}
			}

			break;
		}
		}
	}
}

void VisualizerWindow::clearWindow()
{
	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(_renderer);
}

void VisualizerWindow::drawDanceFloor()
{
	for (const PixelUpdate & update : *_lastUpdate)
	{
		// Note that properties that specify vertical quantities (e.g. yPos and height) are
		// negative because the origin (0, 0) is located at the top left, not the bottom left
		const int xPos   = _originX + _squareSize * update.x;
		const int yPos   = _originY - _squareSize * update.y;
		const int width  = _squareSize;
		const int height = -_squareSize;
		
		SDL_Rect cellRect {xPos, yPos, width, -height};

		SDL_SetRenderDrawColor(_renderer, update.r, update.g, update.b, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(_renderer, &cellRect);
	}
}

// Adapted from:
// https://stackoverflow.com/questions/22886500/how-to-render-text-in-sdl2
void VisualizerWindow::drawWaitingMessage()
{
	constexpr int fontSize = 90;
	const char * fontLocation = "../res/fonts/OpenSans/OpenSans-Regular.ttf";
	TTF_Font * font = TTF_OpenFont(fontLocation, fontSize);
	if (!font)
	{
		ERR("Could not find font: %s", fontLocation);
		exit(-1);
	}
	
	SDL_Color white {255, 255, 255};
	SDL_Surface * surface = TTF_RenderText_Solid(font, "Waiting for frames...", white);

	SDL_Texture * texture = SDL_CreateTextureFromSurface(_renderer, surface);

	SDL_RenderCopy(_renderer, texture, nullptr, nullptr);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	TTF_CloseFont(font);
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
	constexpr int margin = 40;
	
	const double widthScaleFactor  = (double)(_windowWidth  - 2*margin) / (double)ddf.width();
	const double heightScaleFactor = (double)(_windowHeight - 2*margin) / (double)ddf.height();

	_squareSize = round(std::min(widthScaleFactor, heightScaleFactor));
	
	_originX = (_windowWidth - ddf.width() * _squareSize) / 2;
	_originY = _windowHeight - (_windowHeight - ddf.height() * _squareSize) / 2;
}
