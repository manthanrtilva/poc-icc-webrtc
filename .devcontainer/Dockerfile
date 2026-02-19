FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="/depot_tools:${PATH}"
ENV ARCH=x64
ENV TZ=Asia/Kolkata
ENV LANG=en_US.UTF-8

RUN apt-get update && \
    apt-get install -y curl at-spi2-core binutils binutils-aarch64-linux-gnu binutils-arm-linux-gnueabihf binutils-mips64el-linux-gnuabi64 binutils-mipsel-linux-gnu bison bzip2 cdbs curl dbus-x11 devscripts dpkg-dev elfutils fakeroot file flex git git-core gperf lib32z1 libasound2 libasound2-dev libatk1.0-0 libatspi2.0-0 libatspi2.0-dev libbluetooth-dev libbrlapi0.8 libbrlapi-dev libbz2-1.0 libbz2-dev libc6 libc6-dev libcairo2 libcairo2-dev libcap2 libcap-dev libcgi-session-perl libcups2 libcups2-dev libcurl4-gnutls-dev libdrm2 libdrm-dev libegl1 libelf-dev libevdev2 libevdev-dev libexpat1 libffi8 libffi-dev libfontconfig1 libfreetype6 libfuse2 libgbm1 libgbm-dev libgl1 libglib2.0-0 libglib2.0-dev libglu1-mesa-dev libgraphene-1.0-0 libgtk-3-0 libgtk-3-dev libinput10 libinput-dev libjpeg-dev libkrb5-dev libncurses6 libnspr4 libnspr4-dev libnss3 libnss3-dev libpam0g libpam0g-dev libpango-1.0-0 libpangocairo-1.0-0 libpci3 libpci-dev libpcre3 libpixman-1-0 libpng16-16 libpulse0 libpulse-dev libsctp-dev libspeechd2 libspeechd-dev libsqlite3-0 libsqlite3-dev libssl-dev libstdc++6 libsystemd-dev libudev1 libudev-dev libuuid1 libva-dev libvulkan1 libvulkan-dev libwayland-egl1 libwayland-egl1-mesa libwww-perl libx11-6 libx11-xcb1 libxau6 libxcb1 libxcomposite1 libxcursor1 libxdamage1 libxdmcp6 libxext6 libxfixes3 libxi6 libxinerama1 libxkbcommon-dev libxrandr2 libxrender1 libxshmfence-dev libxslt1-dev libxss-dev libxt-dev libxtst6 libxtst-dev lighttpd locales lsb-release mesa-common-dev mutter-common openbox p7zip patch perl pkgconf python3 rpm ruby sudo uuid-dev vim wdiff x11-utils x11-xserver-utils xcompmgr xserver-xorg-core xserver-xorg-video-dummy xvfb xz-utils zip zlib1g zstd

RUN git config --global --add safe.directory /depot_tools && \
    git config --global --add safe.directory /webrtc/src && \
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git /depot_tools && \
    mkdir /workspaces/
COPY webrtc.gclient /workspaces/.gclient
RUN cd /workspaces/ && gclient sync || true
# COPY build.patch /workspaces/src/
# RUN cd /workspaces/src/ && git clone https://github.com/webrtc-sdk/libwebrtc && git apply libwebrtc/patchs/custom_audio_source_m137.patch
# RUN cd /workspaces/src/ && git apply build.patch
RUN cd /workspaces/src/ && bash build/install-build-deps.sh
RUN apt update && apt install -y cmake build-essential gdb
RUN echo "export PATH=/depot_tools:\$PATH" >> ~/.bashrc
COPY 18-02-26.patch /workspaces/src/
RUN cd /workspaces/src/ && git apply 18-02-26.patch
# COPY build.sh /workspaces/src/
# RUN cd /workspaces/src/ && bash build.sh
WORKDIR /workspaces/
CMD ["bash"]