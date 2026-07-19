FROM debian:bookworm

# 安裝編譯工具與開發庫
RUN apt-get update && apt-get install -y \
    build-essential cmake pkg-config \
    libopencv-dev qtbase5-dev \
    libgl1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app