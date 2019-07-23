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

		// TODO Rewrite this abomination into something clearer

		unsigned lastStripNumber = 255;
		const uint8_t * pixelLocation = (const uint8_t *)(frame.data() + 4);
		vector<PixelUpdate> update(ddf.numberOfPixels());

		for (const auto && pixel : ddf)
		{
			if (pixel.stripNumber != lastStripNumber)
			{
				++pixelLocation;
				lastStripNumber = pixel.stripNumber;
			}
			
			uint8_t r = pixelLocation[0];
			uint8_t g = pixelLocation[1];
			uint8_t b = pixelLocation[2];

			INFO("(%u, %u, %u)", r, g, b);

			update.push_back({pixel.pos.x, pixel.pos.y, r, g, b});
			
			pixelLocation += 3;
		}

		//frame.print();

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
