FROM debian:bookworm

ENV DEBIAN_FRONTEND noninteractive

RUN apt update --allow-releaseinfo-change
RUN apt upgrade --yes

RUN apt install --yes build-essential cmake ninja-build pkg-config
RUN apt install --yes qtbase5-dev libqt5websockets5-dev libbotan-2-dev libpq-dev zlib1g-dev

WORKDIR /cflib

COPY . .

WORKDIR /cflib-build

RUN cmake -GNinja /cflib
RUN cmake --build .
# TODO (chris): some tests are failing, fix and enable
#RUN ctest
