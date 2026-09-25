# Multi-stage Dockerfile for rmap (Hardware Register Map Designer & Model Generator)
# Stage 1: Build
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -qq && apt-get install -y -qq --no-install-recommends \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    libqt6test6 \
    libprotobuf-dev \
    protobuf-compiler \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr \
    && cmake --build build -j$(nproc) \
    && cmake --install build

# Stage 2: Minimal Runtime
FROM ubuntu:24.04 AS runtime

LABEL maintainer="Ezequiel Alves <https://github.com/zkalves/rmap>"
LABEL org.opencontainers.image.title="rmap"
LABEL org.opencontainers.image.description="Hardware Register Map Designer & Model Generator"
LABEL org.opencontainers.image.url="https://github.com/zkalves/rmap"
LABEL org.opencontainers.image.source="https://github.com/zkalves/rmap"
LABEL org.opencontainers.image.licenses="MPL-2.0"

ENV DEBIAN_FRONTEND=noninteractive
ENV QT_QPA_PLATFORM=offscreen

RUN apt-get update -qq && apt-get install -y -qq --no-install-recommends \
    libqt6widgets6t64 \
    libqt6gui6t64 \
    libqt6core6t64 \
    qt6-qpa-plugins \
    libprotobuf32t64 \
    python3 \
    make \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy installed files from builder
COPY --from=builder /usr/bin/rmap /usr/bin/rmap
COPY --from=builder /usr/share/rmap /usr/share/rmap
COPY --from=builder /usr/share/applications/rmap.desktop /usr/share/applications/rmap.desktop
COPY --from=builder /usr/share/icons/hicolor /usr/share/icons/hicolor

WORKDIR /work

ENTRYPOINT ["rmap"]
CMD ["--help"]
