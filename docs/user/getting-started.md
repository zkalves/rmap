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
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j$(nproc)
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

---

## 3. Installing rmap

You can install the `rmap` executable to a system path or choose a custom user-defined location.

### Using Make

The `Makefile` supports the standard `PREFIX` variable (defaults to `/usr/local`) and `DESTDIR` for staging:

```bash
# Default installation (/usr/local/bin/rmap)
sudo make install

# Install to custom directory (e.g. ~/.local/bin/rmap)
make install PREFIX=$HOME/.local

# Staged installation for packaging
make install DESTDIR=/tmp/staging PREFIX=/usr

# Uninstall
sudo make uninstall
# or for a custom prefix:
make uninstall PREFIX=$HOME/.local
```

### Using CMake

Choose the installation directory either during CMake configuration or at install time:

```bash
# Option A: Set installation directory during configuration
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build -j$(nproc)
cmake --install build

# Option B: Override destination prefix directly during installation
cmake --install build --prefix $HOME/.local
```

---

## 4. Launching the Application

### Interactive GUI
```bash
# Launch fresh workspace (from build tree or PATH)
rmap
# or: ./build/bin/rmap

# Open sample SPI register map
rmap -f examples/rmt/peripherals/spi.rmt
```

### Batch Headless Code Generation

```bash
rmap -f examples/rmt/peripherals/spi.rmt --export --out ./work
```

[Next: GUI User Guide &rarr;](gui-guide.md)
