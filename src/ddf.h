#ifndef DDF_H
#define DDF_H

#include "dancefloor/DanceFloor.h"
#include "dancefloor/SnakingStrip.h"

// The triple braces are needed, because we're brace-initializing a type
// that is composed of one non-aggregate type (a std::arary).
//
// Think of the inner two braces as denoting a literal std::array
static constexpr int numStrips = 6;
constexpr DanceFloor<SnakingStrip, numStrips> ddf
{{{
	// <start pos>  <width>  <height>  <snaking style>    <position in room>
	{{12,  0},      12,      16,       SNAKE_HORIZONTAL}, // Window right
	{{12, 16},      12,      16,       SNAKE_HORIZONTAL}, // Middle right
	{{12, 32},      12,      16,       SNAKE_HORIZONTAL}, // Near right
	{{ 0,  0},      12,      16,       SNAKE_HORIZONTAL}, // Window left
	{{ 0, 16},      12,      16,       SNAKE_HORIZONTAL}, // Middle left
	{{ 0, 32},      12,      16,       SNAKE_HORIZONTAL}  // Near left
}}};

#endif
