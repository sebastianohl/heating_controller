ARG VARIANT="buster"
FROM mcr.microsoft.com/vscode/devcontainers/base:0-${VARIANT}

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y git wget flex bison gperf python3 python3-pip python3-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util screen
RUN export DEBIAN_FRONTEND=noninteractive && apt-get remove --purge -y python python2.7 && apt-get autoremove --purge -y
RUN ln -s /usr/bin/python3 /usr/bin/python
RUN ln -s /usr/bin/pip3 /usr/bin/pip

RUN mkdir -p /esp
RUN cd /esp && git clone -b v4.1.1 --recursive https://github.com/espressif/esp-idf.git 
RUN su - vscode -c "cd /esp/esp-idf && ./install.sh"
RUN echo ". /esp/esp-idf/export.sh" >> /home/vscode/.profile

RUN export DEBIAN_FRONTEND=noninteractive && apt-get install -y libpython2.7 usbutils



