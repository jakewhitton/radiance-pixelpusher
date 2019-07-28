#ifndef VISUALIZERWINDOW_H
#define VISUALIZERWINDOW_H

#include <SDL2/SDL.h>
#include <optional>
#include "DanceFloorUpdate.h"
#include "misc/Queue.h"

class VisualizerWindow
{
	static constexpr int frameRate = 60;
	
	bool _running;
	Queue<DanceFloorUpdate> & _updateQueue;
	std::optional<DanceFloorUpdate> _lastUpdate;

	SDL_Window * _window;
	SDL_Renderer * _renderer;

	// Spacing parameters
	int _windowWidth, _windowHeight;
	int _originX, _originY;
	int _squareSize;

	void createWindow();
	void handleEvents();
	void setSpacingParameters();
	void clearWindow();
	void drawDanceFloor();
	void drawWaitingMessage();
	void destroyWindow();

public:
	VisualizerWindow(Queue<DanceFloorUpdate> & updateQueue);
	~VisualizerWindow();

	void run();
	void stop();
};

#endif
