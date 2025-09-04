FROM debian:bookworm

ARG DEBIAN_FRONTEND=noninteractive

# Base tools and deps
RUN set -eux; \
    for i in 1 2 3 4 5 6 7 8; do mkdir -p /usr/share/man/man$i; done; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
      ca-certificates \
      ca-certificates-java \
      gnupg \
      lsb-release \
      wget \
      git \
      bash \
      build-essential \
      cmake \
      ninja-build \
      gdb \
      pkg-config \
      python3 \
      curl \
      less \
      unzip \
      castxml \
      libboost-all-dev \
      libdb5.3 \
      libdb5.3-dev \
      libmariadb-dev \
      libmariadb-dev-compat \
      libpcre3 \
      libpcre3-dev \
      libreadline-dev \
      libssl-dev \
      lua5.3 \
      liblua5.3-0 \
      liblua5.3-dev \
      libhiredis-dev \
      openjdk-17-jre-headless; \
    rm -rf /var/lib/apt/lists/*

# Install LLVM/Clang 19 and set it as default clang/clang++
RUN set -eux; \
    echo "deb http://apt.llvm.org/bookworm/ llvm-toolchain-bookworm-19 main" > /etc/apt/sources.list.d/llvm.list; \
    wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc > /dev/null; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
      clang-19 \
      clang-tools-19 \
      lld-19 \
      lldb-19 \
      clang-tidy-19 \
      clang-format-19; \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100; \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100; \
    update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-19 100; \
    update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-19 100; \
    update-alternatives --install /usr/bin/ld.lld ld.lld /usr/bin/ld.lld-19 100; \
    clang --version; \
    clang++ --version; \
    rm -rf /var/lib/apt/lists/*

ENV CC=clang \
    CXX=clang++

WORKDIR /workspaces
