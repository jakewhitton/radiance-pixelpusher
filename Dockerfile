FROM nvidia/opengl:base

# Radiance setup image. INSTALL A LOT
RUN mkdir radiance-pixelpusher
WORKDIR /radiance-pixelpusher/
COPY . .
RUN \
apt-get update && apt-get install --no-install-recommends sudo libsdl2-dev libsdl2-ttf-dev software-properties-common -y && apt-add-repository ppa:ubuntu-toolchain-r/test -y && apt update && apt install --no-install-recommends gcc-9 g++-9  -y && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 30 && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 30 && apt-get install --no-install-recommends python-pip -y && pip install --no-cache-dir --upgrade cmake && export PATH="/usr/local/lib/python2.7/dist-packages:$PATH" && apt-add-repository ppa:beineri/opt-qt591-xenial -y && apt-get update && apt-get install --no-install-recommends qt59base qt59multimedia qt59quickcontrols qt59imageformats qt59quickcontrols2 qt59script libfftw3-dev libsamplerate0-dev libasound2-dev libmpv-dev libdrm-dev libgl1-mesa-dev mesa-utils portaudio19-dev -y && apt clean  && rm -rf /var/lib/apt/lists/* && cd radiance && mkdir build && cd build && cmake .. -DCMAKE_PREFIX_PATH=/opt/qt59 && make && cd ../.. && mkdir build && cd build && cmake .. && make && apt purge software-properties-common -y && apt autoremove -y && apt clean && rm -rf /var/lib/apt/lists/* && rm -rf src CMakeLists.txt radiance/src radiance/CMakeLists.txt

RUN export uid=1000 gid=1000 && \
    mkdir -p /home/developer && \
    echo "developer:x:${uid}:${gid}:Developer,,,:/home/developer:/bin/bash" >> /etc/passwd && \
    echo "developer:x:${uid}:" >> /etc/group && \
    echo "developer ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/developer && \
    chmod 0440 /etc/sudoers.d/developer && \
    chown ${uid}:${gid} -R /home/developer

USER developer
ENV HOME /home/developer
CMD ["/bin/bash"]


