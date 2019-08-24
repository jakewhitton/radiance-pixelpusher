#include "InputEventScanner.h"
#include <unistd.h>

using std::thread;

InputEventScanner::InputEventScanner(Queue<InputEvent> & eventQueue)
	: _eventQueue(eventQueue)
{ }

InputEventScanner::~InputEventScanner()
{
	if (_running)
	{
		stop();
	}
}

void InputEventScanner::start()
{
	_running = true;
	_eventDetectionThread = thread([this]()
	{
		detectEvents();
	});
}

void InputEventScanner::stop()
{
	_running = false;

	if (_eventDetectionThread.joinable())
	{
		_eventDetectionThread.join();
	}
}

bool InputEventScanner::inputDevicesConnected()
{
	// TODO: return whether the controllers are connected so that
	// the game doesn't start until the input devices are available
	return true;
}

void InputEventScanner::detectEvents()
{
	while (_running)
	{
		// TODO: Poll for events
		constexpr unsigned ms = 20;
		usleep(ms * 1000);
	}
}
