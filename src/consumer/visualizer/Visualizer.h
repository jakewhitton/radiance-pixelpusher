#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <thread>
#include "consumer/FrameConsumer.h"
#include "VisualizerWindow.h"
#include "DanceFloorUpdate.h"
#include "misc/Queue.h"

class Visualizer
	: public FrameConsumer
{
	bool _running;
	
	Queue<DanceFloorUpdate> _updateQueue;
	VisualizerWindow _window;
	std::thread _windowThread;

public:
	Visualizer();
	~Visualizer();

	void consumeFrames(Queue<Frame> & frameQueue) override;
	void stop() override;
};

#endif
