#ifndef RADIANCE_H
#define RADIANCE_H

#include <exception>
#include <cstdint>

enum class RadianceCommand
{
	DESCRIPTION              = 0, // Get info about Radiance client
	GET_FRAME                = 1, // Ask for a frame, MUST ISSUE LOOKUP_COORDINATES_2D FIRST
	FRAME                    = 2, // Command for a frame message from Radiance
	LOOKUP_COORDINATES_2D    = 3, // Give uv coordinates that identify pixel locations
	PHYSICAL_COORDINATES_2D  = 4, // Give uv coordinates for radiance to visualize pixel locations
	GEOMETRY_2D              = 5, // Give a png image to visualize pixels against (to see alpha)
	LOOKUP_COORDINATES_3D    = 6, // Not yet supported by Radiance
	PHYSICAL_COORDINATES_3D  = 7, // Not yet supported by Radiance
	GEOMETRY_3D              = 8, // Not yet supported by Radiance
	TUV_MAP                  = 9  // Not yet supported by Radiance
};

class Radiance
{
public:
	static void readRadianceMessage(const int sockfd, RadianceCommand * command, void * dataBuffer, const size_t dataBufferLength, const bool & terminate);
};

#endif
