#include "InputEventScanner.h"
#include <unistd.h>

using std::thread;
wiimote** wiimotes;

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
	int found, connected;
	wiimotes = wiiuse_init(1);

	while (1) {
		printf("[WII-INFO] Searching for wiimotes, enter discovery now\n");
		found = wiiuse_find(wiimotes, 1, 5);
		if (!found) {
			printf("[WII-ERR] Found %i wiimotes out of 1 expected...\n", found);
			exit(1); }

		connected = wiiuse_connect(wiimotes, 1);
		if (connected == 1) {
			printf("[WII-INFO] Connected to all wiimotes!\n"); break; }
		else {
			printf("[WII-ERR] Failed to connect\n"); exit(1); }}
	
	wiiuse_set_leds(wiimotes[0], WIIMOTE_LED_1);
	wiiuse_set_leds(wiimotes[1], WIIMOTE_LED_4);
	wiiuse_rumble(wiimotes[0], 1);
	wiiuse_rumble(wiimotes[1], 1);
	usleep(2000000);
	wiiuse_rumble(wiimotes[0], 0);
	wiiuse_rumble(wiimotes[1], 0);

	return true;
}

void InputEventScanner::detectEvents()
{
	while (_running) {
		if (wiiuse_poll(wiimotes, 1)) {
			switch (wiimotes[0]->event) {
				case WIIUSE_EVENT:
					if (IS_PRESSED(wiimotes[0], WIIMOTE_BUTTON_B)) {
						InputEvent inputEvent;
						inputEvent.type = ROTATE;
						inputEvent.data.rotationDirection = CLOCKWISE;
						_eventQueue.add(inputEvent, 10); } 
					if (IS_PRESSED(wiimotes[0], WIIMOTE_BUTTON_LEFT)) {
						InputEvent inputEvent;
						inputEvent.type = TRANSLATE;
						inputEvent.data.translationDirection = LEFT;
						_eventQueue.add(inputEvent, 10); } 
					if (IS_PRESSED(wiimotes[0], WIIMOTE_BUTTON_DOWN)) {
						InputEvent inputEvent;
						inputEvent.type = TRANSLATE;
						inputEvent.data.translationDirection = DOWN;
						_eventQueue.add(inputEvent, 10); } 
					if (IS_PRESSED(wiimotes[0], WIIMOTE_BUTTON_RIGHT)) {
						InputEvent inputEvent;
						inputEvent.type = TRANSLATE;
						inputEvent.data.translationDirection = RIGHT;
						_eventQueue.add(inputEvent, 10); }
					if (IS_PRESSED(wiimotes[0], WIIMOTE_BUTTON_ONE)) {
						exit(1); } break; }}}


}
