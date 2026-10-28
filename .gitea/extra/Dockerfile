FROM ubuntu:latest

LABEL org.opencontainers.image.authors="Sweetbread" \
      org.opencontainers.image.url="" \
      org.opencontainers.image.documentation="" \
      org.opencontainers.image.source="" \
      org.opencontainers.image.version="" \
      org.opencontainers.image.revision="" \
      org.opencontainers.image.vendor="KolibriOS" \
      org.opencontainers.image.licenses="GPLv2" \
      org.opencontainers.image.title="KolibriOS Toolchain image" \
      org.opencontainers.image.description="Image with predownloaded toolchain and dependencies"

WORKDIR /home/autobuild

RUN apt update
RUN apt install -y \
  perl \
  wget \
  sudo \
  git \
  python3 \
  tree \
  ccache
RUN apt install -y \
  make \
  gcc \
  gcc-multilib \
  g++-multilib \
  build-essential
RUN apt install -y \
  flex \
  bison \
  gperf \
  gawk \
  libfuse3-dev \
  libncurses5-dev \
  libncursesw5-dev \
  libmpfr-dev \
  libgpm-dev \
  zlib1g-dev
RUN apt install -y \
  nasm \
  yasm \
  fasm \
  tup \
  graphviz
RUN apt install -y \
  p7zip-full \
  unzip \
  zip \
  mtools \
  mkisofs \
  parted \
  gdisk

COPY ./install_kgcc .
RUN chmod +x install_kgcc
RUN ./install_kgcc

# Use the newer Ubuntu FASM even when the workflow prepends tools/win32/bin to PATH.
RUN ln -sf /usr/bin/fasm /home/autobuild/tools/win32/bin/fasm

RUN set -eux; \
    wget -q "https://sourceforge.net/projects/jwasm/files/JWasm%20Linux%20binary/JWasm211bl.zip/download" -O /tmp/JWasm211bl.zip; \
    echo "20fe6448f7a6b00d23e5484761e729fba59170ec09d8da8598eaa9081e0c53c4  /tmp/JWasm211bl.zip" | sha256sum -c -; \
    unzip -j /tmp/JWasm211bl.zip jwasm -d /home/autobuild/tools/win32/bin; \
    chmod +x /home/autobuild/tools/win32/bin/jwasm; \
    rm /tmp/JWasm211bl.zip

RUN wget https://git.kolibrios.org/KolibriOS/kolibrios/raw/branch/main/programs/develop/ktcc/bin/kos32-tcc -O /home/autobuild/tools/win32/bin/kos32-tcc
RUN wget https://git.kolibrios.org/KolibriOS/kolibrios/raw/commit/7cf136e2140ccbdad4937ec7b6e0e3f1d16e42de/programs/cmm/c--/c--.elf -O ./tools/win32/bin/c--.elf
RUN echo '/home/autobuild/tools/win32/bin/c--.elf $*' > ./tools/win32/bin/c--
RUN chmod +x ./tools/win32/bin/c--.elf
RUN chmod +x /home/autobuild/tools/win32/bin/c--
RUN chmod +x /home/autobuild/tools/win32/bin/kos32-tcc

# JavaScript actions run with the node executable found in the image PATH.
ARG NODE_VERSION=24.21.0
ARG NODE_SHA256=6e1db87ef58b8819e5d5402eff1536491b18edd8eb7bee5ef7897876e88dc5ff
RUN set -eux; \
    archive="node-v${NODE_VERSION}-linux-x64.tar.gz"; \
    wget -q "https://nodejs.org/dist/v${NODE_VERSION}/${archive}" -O "/tmp/${archive}"; \
    echo "${NODE_SHA256}  /tmp/${archive}" | sha256sum -c -; \
    mkdir -p "/opt/acttoolcache/node/${NODE_VERSION}/x64"; \
    tar -xzf "/tmp/${archive}" -C "/opt/acttoolcache/node/${NODE_VERSION}/x64" --strip-components=1; \
    rm "/tmp/${archive}"
ENV PATH="/opt/acttoolcache/node/${NODE_VERSION}/x64/bin:${PATH}"
RUN node --version && node -e 'if (!globalThis.crypto) process.exit(1)'
