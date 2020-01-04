# Radiance to Pixel Pusher converter

This tool gets frames from a [radiance](https://github.com/zbanks/radiance) instance and writes them to a pixel pusher or a visualizer. 

# Quick start

To avoid compatibility troubles, we've containerized both radiance and this converter into two discrete images. You can run these via docker on Linux (and potentially other platforms as well) by mounting an X11 socket, using the host network for docker, and mounting /dev/snd within the container. 

See the [releases](https://github.com/jakewhitton/radiance-pixelpusher/releases) page to download both images. Then you can either spin up your own docker instance as specified above, or run via [x11docker](https://github.com/mviereck/x11docker) as follows (via sudo if necessary):

```shell
docker load < radiance_latest.tar.gz
docker load < pixelpusher_latest.tar.gz
x11docker --gpu -- --net=host pixelpusher
x11docker --gpu --alsa=PCH -- --net=host radiance
```

# How to build

Make sure you've cloned the git submodules by issuing the following:

```shell
git submodule update --init --recursive
```

You'll need the sdl2 library and a recent version of GCC, as well as the sdl2-ttf library, to compile the visualizer properly. On ubuntu, you can install it with the following:

```shell
sudo apt update && sudo apt install libsdl2-dev libsdl2-ttf-dev
```

Then, navigate to the base of the repo (where this README.md is) and do the following:

```shell
mkdir build
cd build
cmake ..
make
./run.exe
```

# Configuration

The file `src/config.h` defines some values to modify the functioning of the tool, including what port the radiance server should listen to, what address the pixel pusher is located at, and what port the pixel pusher update protocol should use.

Any reconfiguration requires a recompilation to take effect.
