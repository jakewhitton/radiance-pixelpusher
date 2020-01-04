# Convenience!
WORKDIR=$(pwd)
cd build/
./run.exe &

cd $WORKDIR/radiance/build
./radiance
