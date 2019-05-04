#ifndef LUXSERVER_H
#define LUXSERVER_H

#include "BlockingCollection.h"
#include "Frame.h"

class LuxServer
{
	code_machina::BlockingQueue<Frame> & _queue;
	Frame getNewFrame();

public:
	LuxServer(code_machina::BlockingQueue<Frame> & queue);

	void operator()();
};

#endif
