FROM rust:latest
MAINTAINER Felix Leupold <felix@gnosis.pm>

RUN useradd -ms /bin/bash docker
USER docker

COPY --chown=docker:docker . /home/docker/bellman
WORKDIR /home/docker/bellman
