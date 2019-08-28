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
	wiimote** wiimotes; int found, connected;
	wiimotes = wiiuse_init(2);

	while (1) {
		printf("[WII-INFO] Searching for wiimotes, enter discovery now\n");
		found = wiiuse_find(wiimotes, 2, 10);
		if (found != 2) {
			printf("[WII-ERR] Found %i wiimotes out of 2 expected...\n", found);
			wiiuse_cleanup(wiimotes, 2); continue; }

		connected = wiiuse_connect(wiimotes, 2);
		if (connected = 2) {
			printf("[WII-INFO] Connected to all wiimotes!\n"); break; }
		else {
			printf("[WII-ERR] Failed to connect\n"); wiiuse_cleanup(wiimotes, 2);
			continue; }}
	
	wiiuse_set_leds(wiimotes[0], WIIMOTE_LED_1);
	wiiuse_set_leds(wiimotes[1], WIIMOTE_LED_4);
	wiiuse_rumble(wiimotes[0], 1);
	wiiuse_rumble(wiimotes[1], 1);
	usleep(200000);
	wiiuse_rumble(wiimotes[0], 0);
	wiiuse_rumble(wiimotes[1], 0);

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
