FROM nvidia/opengl:1.0-glvnd-devel

# Radiance setup image. INSTALL A LOT
RUN mkdir radiance-pixelpusher
WORKDIR /radiance-pixelpusher/
COPY . .
RUN \
apt-get update && apt-get install --no-install-recommends libsdl2-dev libsdl2-ttf-dev software-properties-common -y && apt-add-repository ppa:ubuntu-toolchain-r/test -y && apt update && apt install --no-install-recommends gcc-9 g++-9  -y && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 30 && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 30 && apt-get install --no-install-recommends python-pip -y && pip install --no-cache-dir --upgrade cmake && export PATH="/usr/local/lib/python2.7/dist-packages:$PATH" && apt-add-repository ppa:beineri/opt-qt591-xenial -y && apt-get update && apt-get install -y qt59base qt59graphicaleffects qt59multimedia qt59quickcontrols qt59imageformats qt59quickcontrols2 qt59script && apt-get install --no-install-recommends libfftw3-dev libsamplerate0-dev libgl1-mesa-dri libasound2-dev libmpv-dev libdrm-dev libgl1-mesa-dev mesa-utils portaudio19-dev -y && apt clean  && rm -rvf /var/lib/apt/lists/* && cd radiance && mkdir build && cd build && cmake .. -DCMAKE_PREFIX_PATH=/opt/qt59 && make -j4 && cd ../.. && mkdir build && cd build && cmake .. && make -j4 && apt purge software-properties-common -y && apt autoremove -y && apt clean && rm -rvf /var/lib/apt/lists/* && cd /radiance-pixelpusher/ && rm -rvf src CMakeLists.txt radiance/src radiance/CMakeLists.txt

WORKDIR /radiance-pixelpusher/radiance/build/

CMD ["./radiance"]

