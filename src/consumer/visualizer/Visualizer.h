#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <SDL.h>
#include "consumer/FrameConsumer.h"

class Visualizer
	: public FrameConsumer
{

	SDL_Window * window;

	SDL_Surface * screenSurface;

    SDL_Renderer * screenRenderer;

private:
    bool _running;
    void fillSquare(int row, int col, uint8_t r, uint8_t g, uint8_t b);
    bool _sdlInitialized;
    void initializeSDL();
    const int TEST_WIDTH = 12;
    const int TEST_HEIGHT = 16;

public:
    constexpr static const int SQUARE_SIZE = 60;
	Visualizer();
	~Visualizer();

	void consumeFrames(FrameQueue & frameQueue) override;
	void stop() override;
};

#endif
