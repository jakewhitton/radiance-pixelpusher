# Docker build files

These are used to build the docker images for both the pixelpusher tool and radiance itself. To use, move these build files into the root directory of this repo, and do the following as root.

```shell
docker build . -t radiance_image -f Dockerfile.radiance
docker build . -t pixelpusher_layer_image -f Dockerfile.compat
```

