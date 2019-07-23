#ifndef VISUALIZERWINDOW_H
#define VISUALIZERWINDOW_H

#include <SDL.h>
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
	bool _refresh;

	// Spacing parameters
	int _originX, _originY;
	int _squareSize;

	void createWindow();
	void processEvents();
	void destroyWindow();

	void setSpacingParameters();
	SDL_Rect getCellRect(int x, int y);
	void fillCell(int x, int y, uint8_t r, uint8_t g, uint8_t b);
public:
	VisualizerWindow(Queue<DanceFloorUpdate> & updateQueue);
	~VisualizerWindow();

	void run();
	void stop();
};

#endif
