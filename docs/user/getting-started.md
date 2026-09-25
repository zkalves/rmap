# Getting Started

This guide explains how to install prerequisites, build **rmap**, and run your first register map project.

---

## 1. Prerequisites

**rmap** is built with **C++17** and modern CMake.

### Dependencies
- Modern C++ compiler (`g++` 9+ or `clang++` 10+)
- CMake 3.15+
- Qt 6 (`QtCore`, `QtWidgets`, `QtTest`) *(required for GUI, headless offscreen CLI operations, and test runner)*
- Google Protocol Buffers (`protobuf-compiler`, `libprotobuf-dev`)

### Installation Commands

#### Ubuntu / Debian (22.04 / 24.04 / Debian 12+)
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

#### Fedora / RHEL 9 / Rocky Linux 9 / AlmaLinux 9 / CentOS Stream 9
```bash
sudo dnf install -y \
  gcc-c++ \
  cmake \
  git \
  qt6-qtbase-devel \
  qt6-qttools-devel \
  protobuf-devel \
  protobuf-compiler \
  rpm-build
```

#### RHEL 8 / Rocky Linux 8 / AlmaLinux 8
RHEL 8 requires **EPEL 8** for Qt 6 and **GCC Toolset 11** for Modern C++17:
```bash
# Enable EPEL and PowerTools / CRB
sudo dnf install -y epel-release dnf-plugins-core
sudo dnf config-manager --set-enabled powertools  # or 'crb' on Rocky/Alma 8

# Install C++17 compiler toolset and Qt 6
sudo dnf install -y \
  gcc-toolset-11-gcc-c++ \
  cmake \
  git \
  qt6-qtbase-devel \
  protobuf-devel \
  protobuf-compiler \
  rpm-build

# Activate GCC 11 in current shell session before compiling
source /opt/rh/gcc-toolset-11/enable
```

#### openSUSE (Leap 15.5+ / Tumbleweed) & SLES 15 (SP4+)
```bash
# On SLES 15, ensure SUSE Package Hub or Developer Module is active
sudo zypper install -y \
  gcc-c++ \
  cmake \
  git \
  qt6-base-devel \
  protobuf-devel \
  rpm-build
```

#### macOS (Homebrew)
```bash
brew install cmake qt@6 protobuf
brew link qt@6
```

---

> [!TIP]
> **Supported Platforms & Compatibility Matrix**:
> For the complete Linux distribution compatibility matrix, glibc baselines, pre-built packages (AppImage, DEB, RPM), and macOS/Windows details, see the dedicated [Supported Operating Systems & Compatibility Matrix Guide](supported-os.md).

---

## 2. Building rmap

**rmap** provides a top-level `Makefile` wrapper around standard CMake commands:

```bash
# Clone repository
git clone https://github.com/zkalves/rmap.git
cd rmap

# Compile all targets and test suites (with Qt GUI)
make

# Compile application binary only (fast, skips test targets)
make rmap

# Run the test suite
make test

# Run unit tests with compiler code coverage and print metrics summary
make coverage

# Generate HTML, Markdown, and JSON coverage reports
make coverage-report

# Launch the application
make run
```

### Direct CMake Commands

#### Standard Build (Qt GUI Enabled)
```bash
# Build and run tests
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j$(nproc)
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure

# Or build with multi-metric code coverage enabled
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON
cmake --build build -j$(nproc)
python3 script/generate_coverage.py --build-dir build --summary
```

#### Headless & CI/CD Execution (Offscreen Platform)
For headless CI environments, compute nodes, or remote server systems where no display server (X11 / Wayland) is running, execute `rmap` headlessly using the Qt offscreen platform plugin:

```bash
# Execute headless verification or export offscreen
QT_QPA_PLATFORM=offscreen ./build/bin/rmap --version
QT_QPA_PLATFORM=offscreen ./build/bin/rmap --help
```


---

## 3. Installing from Source

You can install the `rmap` executable built from source to a system path or custom user directory.

> [!TIP]
> **Pre-built Packages**:
> If you prefer pre-compiled binaries instead of compiling from source, standalone **AppImage**, **DEB**, and **RPM** packages are available. See the [Supported Operating Systems & Compatibility Matrix Guide](supported-os.md) for installation instructions and local packaging commands (`make package-deb`, `make package-rpm`, `make package-appimage`).

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

---

## 5. Verifying Installation with Bundled Examples

**rmap** packages a rich suite of reference register maps and self-contained simulation environments in `<prefix>/share/rmap/examples` (and bundled templates in `<prefix>/share/rmap/templates`).

Users can copy the `examples` folder to any workspace and run the test targets directly to verify their installation:

```bash
# Copy examples to a working directory
cp -r /usr/local/share/rmap/examples ~/my_rmap_examples
# (or for custom install prefix: cp -r ~/.local/share/rmap/examples ~/my_rmap_examples)

cd ~/my_rmap_examples

# Run full simulation and compilation across all example environments
make all

# Or test an individual peripheral environment (e.g. SPI)
cd environments/spi
make all
make compile-c
make run-python
make sim-rtl
make clean
```

All environment Makefiles automatically discover the installed `rmap` executable from `PATH` and resolve the installed templates, requiring zero configuration.

---

[Next: Supported Operating Systems &rarr;](supported-os.md) | [GUI User Guide &rarr;](gui-guide.md)
