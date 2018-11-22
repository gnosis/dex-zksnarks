FROM pepper-uxio
MAINTAINER Felix Leupold <felix@gnosis.pm>

ENV PEPPER /opt/pequin/

RUN apt-get update && apt-get install -y inotify-tools libgtest-dev time vim tmux
RUN cd /usr/src/gtest && cmake CMakeLists.txt && make && cp *.a /usr/lib

COPY . /opt/dex-zksnarks
RUN cd /opt/dex-zksnarks && make initial && cd $PEPPER/pepper

# Watch makefile for changes in the background and start a shell
ENTRYPOINT (cd /opt/dex-zksnarks && \
	(while inotifywait -qq -e modify Makefile; do \
	 	make -s all && tput bel; \
	done)&) && \
	/bin/bash
