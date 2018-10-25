FROM fleupold/pepper
MAINTAINER Felix Leupold <felix@gnosis.pm>

ENV PEPPER /home/docker/pequin/

USER root
RUN apt-get install -y inotify-tools
USER docker

COPY . /home/docker/dex-zksnarks
WORKDIR /home/docker/dex-zksnarks

RUN make initial

WORKDIR /home/docker/pequin/pepper/

# Watch makefile for changes in the background and start a shell
ENTRYPOINT (cd /home/docker/dex-zksnarks && \
	(while inotifywait -qq -e modify Makefile; do \
	 	make -s all && tput bel; \
	done)&) && \
	/bin/bash