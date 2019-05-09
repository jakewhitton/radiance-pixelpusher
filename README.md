# Radiance to Pixel Pusher converter

This tool gets frames from a [radiance](https://github.com/zbanks/radiance) instance and writes them to a pixel pusher.

# How to build

Make sure you've cloned the git submodules by issuing the following:

```shell
git submodule update --init
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

The file `src/conig.h` defines some values to modify the functioning of the tool, including what port the radiance server should listen to, what address the pixel pusher is located at, and what port the pixel pusher update protocol should use.

Any reconfiguration requires a recompilation to take effect.
