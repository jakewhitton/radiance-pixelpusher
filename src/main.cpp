#include <iostream>
#include "producer/radiance/LuxServer.h"
#include "consumer/pixelpusher/PixelPusher.h"
#include "producerconsumerprogram/ProducerConsumerProgram.h"
#include "misc/Log.h"

using std::cin;

int main()
{

	INFO("Issue an EOF (Ctrl + D) to terminate...");

	const char * radiancePort = "11647";
	LuxServer producer {radiancePort};

	const char * pixelPusherLocation = "192.168.0.5";
	const char * pixelPusherPort = "9897";
	PixelPusher consumer {pixelPusherLocation, pixelPusherPort};

	ProducerConsumerProgram program {producer, consumer};

	program.start();

	// Spin until EOF is detected
	cin.get();
	while (!cin.eof())
	{
		cin.get();
	}

	program.stop();
}
