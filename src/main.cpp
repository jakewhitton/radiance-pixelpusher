#include <iostream>
#include <thread>
#include "radiance/LuxServer.h"
#include "pixelpusher/PixelPusherClient.h"
#include "pixelpusher/Frame.h"
#include "misc/Log.h"

using std::cin;
using std::thread;

int main()
{

	INFO("Issue an EOF (Ctrl + D) to terminate...");

	// Queue for producer-consumer pattern
	const int QUEUE_SIZE = 10;
	PixelPusherClient::queue_t queue(QUEUE_SIZE);

	INFO("Initializing Lux client");
	LuxServer luxServer {queue};

	INFO("Initializing Pixel Pusher client");
	PixelPusherClient pixelPusherClient {queue};

	thread luxServerThread {std::ref(luxServer)};                 // automatically starts
	thread pixelPusherClientThread {std::ref(pixelPusherClient)}; // automatically starts

	// Spin until EOF is detected
	cin.get();
	while (!cin.eof())
	{
		cin.get();
	}

	// Make the queue unable to accept new frames
	//
	// The code for LuxServer and PixelPusherClient listen for the completion
	// of the queue and the processing of the last item in a completed queue,
	// respectively, as signals to safely terminate their threads.
	queue.complete_adding();

	// Join with threads and then terminate program
	//
	// Note : not doing this just causes a crash because terminating the main thread
	// while other threads are active causes the execution of the other threads to
	// stop abruptly, leaving them no time to clean up things like file descriptors
	// for sockets or even calling object destructors).
	luxServerThread.join();
	pixelPusherClientThread.join();
}
