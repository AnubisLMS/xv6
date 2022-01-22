FROM debian:buster-slim

ENV TOOLPREFIX=""

RUN apt update \
  && apt install -y make gcc libc6-i386 \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp/build
