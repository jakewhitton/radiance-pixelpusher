#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <SDL.h>
#include "consumer/FrameConsumer.h"

class Visualizer
	: public FrameConsumer
{

	SDL_Window * window;

	SDL_Surface * screenSurface;

public:
	Visualizer();
	~Visualizer();

	void consumeFrames(FrameQueue & frameQueue) override;
	void stop() override;
};

#endif
