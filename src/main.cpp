#include <iostream>
#include "producer/radiance/Radiance.h"
#include "producer/tetris/Tetris.h"
#include "consumer/pixelpusher/PixelPusher.h"
#include "consumer/visualizer/Visualizer.h"
#include "dancefloor/DanceFloorProgram.h"
#include "misc/Log.h"

using std::cin;

int main()
{
	INFO("Issue an EOF (Ctrl + D) to terminate...");

	//const char * radiancePort = "11647";
	//Radiance producer {radiancePort};
	
	Tetris producer;
	
	//const char * pixelPusherLocation = "192.168.0.177";
	//const char * pixelPusherPort = "9897";
	//PixelPusher consumer {pixelPusherLocation, pixelPusherPort};
	
	Visualizer consumer;

	DanceFloorProgram program {producer, consumer};

	program.start();

	// Spin until EOF is detected
	cin.get();
	while (!cin.eof())
	{
		cin.get();
	}

	program.stop();
}
