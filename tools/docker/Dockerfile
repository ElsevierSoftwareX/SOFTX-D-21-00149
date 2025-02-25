FROM i386/ubuntu:18.04

ENV DEBIAN_FRONTEND noninteractive

# Set user for what comes next
USER root

# Tools
RUN apt-get -qq update && \
    apt-get -qq -y --no-install-recommends install gnupg ca-certificates software-properties-common > /dev/null

RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF && \
  echo "deb http://download.mono-project.com/repo/ubuntu stable-bionic main" | tee /etc/apt/sources.list.d/mono-xamarin.list && \
  add-apt-repository ppa:mosquitto-dev/mosquitto-ppa && \
  apt-get -qq update && \
  apt-get -qq -y --no-install-recommends install \
    ant \
    build-essential \
    doxygen \
    gdb \
    git \
    gtk-sharp2 \
    iputils-tracepath \
    iputils-ping \
    libcanberra-gtk-module:i386 \
    libcoap-1-0-bin \
    libfreetype6-dev \
    libgtk2.0-0 \
    libpng-dev \
    mono-complete \
    mosquitto \
    mosquitto-clients \
    net-tools \
    openjdk-11-jdk \
    python-dev \
    python3-dev \
    python-pip \
    python3-pip \
    python3-setuptools \
    python-serial \
    python3-serial \
    rlwrap \
    sudo \
    screen \
    srecord \
    uml-utilities \
    unzip \
    libusb-1.0-0 \
    valgrind \
    wget \
    smitools \
    snmp \
    snmp-mibs-downloader \
    > /dev/null \
  && apt-get -qq clean

# Install ARM toolchain
RUN wget -nv https://github.com/Yagoor/gcc-arm-none-eabi/releases/download/9-2020-q2-update/gcc-arm-none-eabi-9-2020-q4-major-i386-linux.tar.bz2 && \
  tar xjf gcc-arm-none-eabi-9-2020-q4-major-i386-linux.tar.bz2 -C /tmp/ && \
  cp -f -r /tmp/gcc-arm-none-eabi-9-2020-q4-major/* /usr/local/ && \
  rm -rf /tmp/gcc-arm-none-eabi-* gcc-arm-none-eabi-*-linux.tar.bz2

# Install msp430 toolchain
RUN wget -nv http://simonduq.github.io/resources/mspgcc-4.7.2-compiled.tar.bz2 && \
  tar xjf mspgcc*.tar.bz2 -C /tmp/ && \
  cp -f -r /tmp/msp430/* /usr/local/ && \
  rm -rf /tmp/msp430 mspgcc*.tar.bz2

# Install NXP toolchain (partial, with binaries excluded. Download from nxp.com)
RUN wget -nv http://simonduq.github.io/resources/ba-elf-gcc-4.7.4-part1.tar.bz2 && \
  wget -nv http://simonduq.github.io/resources/ba-elf-gcc-4.7.4-part2.tar.bz2 && \
  wget -nv http://simonduq.github.io/resources/jn516x-sdk-4163-1416.tar.bz2 && \
  mkdir /tmp/jn516x-sdk /tmp/ba-elf-gcc && \
  tar xjf jn516x-sdk-*.tar.bz2 -C /tmp/jn516x-sdk && \
  tar xjf ba-elf-gcc-*part1.tar.bz2 -C /tmp/ba-elf-gcc && \
  tar xjf ba-elf-gcc-*part2.tar.bz2 -C /tmp/ba-elf-gcc && \
  cp -f -r /tmp/jn516x-sdk /usr/ && \
  cp -f -r /tmp/ba-elf-gcc /usr/ && \
  rm -rf jn516x*.bz2 ba-elf-gcc*.bz2 /tmp/ba-elf-gcc* /tmp/jn516x-sdk*

ENV PATH="/usr/ba-elf-gcc/bin:${PATH}"

## Install nRF52 SDK
RUN wget -nv https://developer.nordicsemi.com/nRF5_IoT_SDK/nRF5_IoT_SDK_v0.9.x/nrf5_iot_sdk_3288530.zip && \
    mkdir /usr/nrf52-sdk && \
    unzip -q nrf5_iot_sdk_3288530.zip -d /usr/nrf52-sdk && \
    rm nrf5_iot_sdk_3288530.zip

ENV NRF52_SDK_ROOT /usr/nrf52-sdk

# Install nRF Command Line tools
RUN wget -nv https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-12-1/nRFCommandLineTools10121Linuxi386.tar.gz && \
  mkdir /tmp/nrf-cli-tools && \
  tar xzf nRFCommandLineTools10121Linuxi386.tar.gz -C /tmp/nrf-cli-tools && \
  dpkg -i /tmp/nrf-cli-tools/JLink_Linux_V688a_i386.deb && \
  dpkg -i /tmp/nrf-cli-tools/nRF-Command-Line-Tools_10_12_1_Linux-i386.deb && \
  ln -s /opt/SEGGER/JLink_V688a/libjlinkarm.so /opt/SEGGER/JLink_V688a/libjlinkarm_x86.so && \
  rm -rf nRFCommandLineTools10121Linuxi386.tar.gz /tmp/nrf-cli-tools

# Install sphinx and sphinx_rtd_theme, required for building and testing the
# readthedocs API documentation
RUN pip -q install --upgrade pip
RUN pip -q install setuptools && pip -q install sphinx_rtd_theme sphinx
# Install matplotlib for result visualization
RUN pip3 -q install --upgrade pip
RUN pip3 -q install matplotlib

# Install nrfutil and work around broken pc_ble_driver_py dependency
RUN pip3 -q install nrfutil && \
  pip3 -q install --no-deps -t /usr/local/lib/python3.6/dist-packages --python-version 3.6 --ignore-requires-python --upgrade nrfutil

# Make sure we're using Java 11 for Cooja
RUN update-java-alternatives --set /usr/lib/jvm/java-1.11.0-openjdk-i386

# Create user, enable X forwarding, add to group dialout
# -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix
RUN export uid=1000 gid=1000 && \
    mkdir -p /home/user && \
    echo "user:x:${uid}:${gid}:user,,,:/home/user:/bin/bash" >> /etc/passwd && \
    echo "user:x:${uid}:" >> /etc/group && \
    echo "user ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers && \
    chmod 0440 /etc/sudoers && \
    chown ${uid}:${gid} -R /home/user && \
    usermod -aG dialout user

# Set user for what comes next
USER user

# Environment variables
ENV JAVA_HOME           /usr/lib/jvm/java-11-openjdk-i386/
ENV HOME                /home/user
ENV CONTIKI_NG          ${HOME}/contiki-ng
ENV COOJA               ${CONTIKI_NG}/tools/cooja
ENV                     PATH="${HOME}:${PATH}"
ENV                     LC_ALL=C.UTF-8
ENV                     LANG=C.UTF-8
WORKDIR                 ${HOME}

# Create Cooja shortcut
RUN echo "#!/bin/bash\nant -Dbasedir=${COOJA} -f ${COOJA}/build.xml run" > ${HOME}/cooja && \
  chmod +x ${HOME}/cooja

# Download, build and install Renode
RUN git clone --quiet https://github.com/renode/renode.git \
  && cd ${HOME}/renode \
  && git checkout v1.3 \
  && ./build.sh
ENV PATH="${HOME}/renode:${PATH}"

# By default, we use a Docker bind mount to share the repo with the host,
# with Docker run option:
# -v <HOST_CONTIKI_NG_ABS_PATH>:/home/user/contiki-ng
# Alternatively, uncomment the next two lines to download Contiki-NG and pre-compile Cooja.
#RUN git clone --recursive https://github.com/contiki-ng/contiki-ng.git ${CONTIKI_NG}
#RUN ant -q -f ${CONTIKI_NG}/tools/cooja/build.xml jar

# Working directory
WORKDIR ${CONTIKI_NG}

# Start a bash
CMD bash --login
