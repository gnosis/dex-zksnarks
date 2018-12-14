FROM fleupold/pepper:v4
MAINTAINER Felix Leupold <felix@gnosis.pm>

ENV PEPPER /home/docker/pequin

USER root
RUN apt-get install -y inotify-tools libgtest-dev time vim tmux software-properties-common

# Upgrade Boost
RUN apt-get remove -y libboost1.54-dev
RUN add-apt-repository -y ppa:mhier/libboost-latest
RUN apt-get update -y
RUN apt-get install -y libboost1.67-dev

RUN cd /usr/src/gtest && cmake CMakeLists.txt && make && cp *.a /usr/lib

USER docker

COPY --chown=docker:docker . /home/docker/dex-zksnarks
WORKDIR /home/docker/dex-zksnarks

RUN make initial

WORKDIR /home/docker/pequin/pepper/

# Watch makefile for changes in the background and start a shell
ENTRYPOINT (cd /home/docker/dex-zksnarks && \
	(while inotifywait -qq -e modify Makefile; do \
	 	make -s all && tput bel; \
	done)&) && \
	/bin/bash
