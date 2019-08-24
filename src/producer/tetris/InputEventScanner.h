#ifndef INPUTEVENTSCANNER_H
#define INPUTEVENTSCANNER_H

#include "misc/Queue.h"
#include "Types.h"
#include <thread>

enum InputEventType
{
	TRANSLATE,
	ROTATE
};

struct InputEvent
{
	InputEventType type;

	union
	{
		TranslationDirection translationDirection;
		RotationDirection    rotationDirection;
	} data;
};

class InputEventScanner
{
	bool _running = true;
	Queue<InputEvent> & _eventQueue;
	std::thread _eventDetectionThread;

	void detectEvents();
public:
	InputEventScanner(Queue<InputEvent> & eventQueue);
	~InputEventScanner();

	void start();
	void stop();

	bool inputDevicesConnected();
};

#endif
