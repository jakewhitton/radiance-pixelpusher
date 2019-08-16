#include "Visualizer.h"
#include "ddf.h"
#include "misc/Log.h"

using std::vector;
using std::thread;

Visualizer::Visualizer()
	: _running(true)
	, _window(_updateQueue)
{
	_windowThread = thread([this]()
	{
		_window.run();
	});
}

Visualizer::~Visualizer()
{
	_window.stop();
	_windowThread.join();
}

void Visualizer::consumeFrames(Queue<Frame> & frameQueue)
{
	while (_running)
	{
		Frame frame;
		try
		{
			frameQueue.take(frame, _running, false); // Throw if _running == false
		}
		catch (OperationInterruptedException e)
		{
			break;
		}

		constexpr int width  = ddf.width();
		constexpr int height = ddf.height();

		auto pixelLocation = (const uint8_t (*)[height][3])frame.data();
		DanceFloorUpdate update(ddf.numberOfPixels());

		for (int x = 0; x < width; ++x)
		{
			for (int y = 0; y < height; ++y)
			{
				uint8_t r = pixelLocation[x][y][0];
				uint8_t g = pixelLocation[x][y][1];
				uint8_t b = pixelLocation[x][y][2];

				update.push_back({x, y, r, g, b});
   			}
		}

		try
		{
			_updateQueue.add(update, _running, false); // Throw when _running = false
		}
		catch (OperationInterruptedException e)
		{
			break;
		}
	}
}

void Visualizer::stop()
{
	_running = false;
}
