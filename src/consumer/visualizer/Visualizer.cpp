#include "Visualizer.h"
#include "misc/Log.h"

Visualizer::Visualizer()
{
	int status = SDL_Init(SDL_INIT_VIDEO);
	if (status < 0)
	{
		ERR("Could not initialize SDL: %s", SDL_GetError());
		exit(1);
	}

	window = SDL_CreateWindow("Hello",
	                          SDL_WINDOWPOS_UNDEFINED,
	                          SDL_WINDOWPOS_UNDEFINED,
	                          800,
				  400,
				  SDL_WINDOW_SHOWN);
	
	if (!window)
	{
		ERR("Could not create an SDL window: %s", SDL_GetError());
	}

	screenSurface = SDL_GetWindowSurface(window);

	SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

	SDL_UpdateWindowSurface(window);
}

Visualizer::~Visualizer()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Visualizer::consumeFrames(FrameQueue & frameQueue)
{

}

void Visualizer::stop()
{

}
