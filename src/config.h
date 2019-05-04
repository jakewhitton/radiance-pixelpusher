#ifndef CONFIG_H
#define CONFIG_H

// LuxServer config
constexpr const char * LUXSERVER_PORT = "9001"; // Radiance default

// PixelPusherClient config
constexpr const char * PIXELPUSHER_PORT      = "7331";
constexpr const char * PIXELPUSHER_LOCATION  = "1.1.1.2"; // Can be IP or hostname

// Dance floor config
constexpr const int DANCE_FLOOR_WIDTH  = 6;
constexpr const int DANCE_FLOOR_HEIGHT = 5;

#endif
