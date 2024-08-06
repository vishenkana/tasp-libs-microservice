FROM tasp/libcommon:1.0.2

COPY . /microservice
WORKDIR /microservice

RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && apt-get install -y --no-install-recommends --reinstall \
        libevent-dev

RUN mkdir build && cd build && cmake .. && ninja install
