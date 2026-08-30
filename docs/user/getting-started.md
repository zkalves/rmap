# Getting Started

This guide explains how to install prerequisites, build **rmap**, and run your first register map project.

---

## 1. Prerequisites

**rmap** is built with **C++17** and **Qt 6**.

### Dependencies
- Modern C++ compiler (`g++` 9+ or `clang++` 10+)
- CMake 3.15+
- Qt 6 (`QtCore`, `QtWidgets`, `QtTest`)
- Google Protocol Buffers (`protobuf-compiler`, `libprotobuf-dev`)

### Installation Commands

#### Ubuntu / Debian (22.04 / 24.04)
```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  git \
  qt6-base-dev \
  libqt6test6 \
  libprotobuf-dev \
  protobuf-compiler
```

#### macOS (Homebrew)
```bash
brew install cmake qt@6 protobuf
brew link qt@6
```

---

## 2. Building rmap

**rmap** provides a top-level `Makefile` wrapper around standard CMake commands:

```bash
# Clone repository
git clone https://github.com/zkalves/rmap.git
cd rmap

# Compile all targets and test suites
make

# Compile application binary only (fast, skips test targets)
make rmap

# Run the test suite
make test

# Launch the application
make run
```

### Direct CMake Commands
If you prefer direct CMake commands:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j$(nproc)
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

---

## 3. Launching the Application

### Interactive GUI
```bash
# Launch fresh workspace
./build/bin/rmap

# Open sample SPI register map
./build/bin/rmap -f examples/spi.rmt
```

### Headless CLI Export
```bash
./build/bin/rmap -f examples/spi.rmt --export --out ./work
```

[Next: GUI User Guide &rarr;](gui-guide.html)
