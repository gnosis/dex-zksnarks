FROM node:7

MAINTAINER Felix Leupold <felix@gnosis.pm>

RUN useradd -m zokrates

ARG LIBSNARK_COMMIT=f7c87b88744ecfd008126d415494d9b34c4c1b20
ENV LIBSNARK_SOURCE_PATH=/home/zokrates/libsnark-$LIBSNARK_COMMIT

ARG ZOKRATES_COMMIT=dex-snark-master
ENV ZOKRATES_SOURCE_PATH=/home/zokrates/ZoKrates

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    curl \
    libboost-all-dev \
    libgmp3-dev \
    libprocps3-dev \
    libssl-dev \
    pkg-config \
    python-markdown \
    git

USER zokrates

RUN curl https://sh.rustup.rs -sSf | \
    sh -s -- --default-toolchain nightly -y

ENV PATH=/home/zokrates/.cargo/bin:$PATH

RUN git clone https://github.com/scipr-lab/libsnark.git $LIBSNARK_SOURCE_PATH
RUN git clone https://github.com/fleupold/ZoKrates.git $ZOKRATES_SOURCE_PATH

WORKDIR $LIBSNARK_SOURCE_PATH
RUN git checkout $LIBSNARK_COMMIT
RUN git submodule update --init --recursive

WORKDIR $ZOKRATES_SOURCE_PATH
RUN git checkout $ZOKRATES_COMMIT
RUN cargo build --release

WORKDIR /home/zokrates
COPY --chown=zokrates:zokrates . dex-snark

RUN cd dex-snark \
    && npm install
