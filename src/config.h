#ifndef CONFIG_H
#define CONFIG_H

// LuxServer config
constexpr const char * LUXSERVER_PORT = "9001"; // Radiance default

// PixelPusherClient config
constexpr const char * PIXELPUSHER_PORT      = "9897";
constexpr const char * PIXELPUSHER_LOCATION  = "192.168.0.5"; // Can be IP or hostname

// Dance floor config
constexpr const int DANCE_FLOOR_WIDTH  = 12;
constexpr const int DANCE_FLOOR_HEIGHT = 16;

#endif
