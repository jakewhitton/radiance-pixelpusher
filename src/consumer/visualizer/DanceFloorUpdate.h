#ifndef DANCEFLOORSUPDATE_H
#define DANCEFLOORSUPDATE_H

#include <vector>
#include "ddf.h"

struct PixelUpdate
{
	int x;
	int y;

	uint8_t r;
	uint8_t g;
	uint8_t b;
};

using DanceFloorUpdate = std::vector<PixelUpdate>;

#endif
