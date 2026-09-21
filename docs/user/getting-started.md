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

### Supported Linux Distributions & Compatibility Matrix

| Distribution Family | Earliest Supported Version | glibc | Status & Repositories | Supported Deliverables |
| :--- | :--- | :--- | :--- | :--- |
| **RHEL / Rocky / AlmaLinux 9** | **9.0+** (Full support) | 2.34 | **Tier-1 Native**: Default AppStream includes Qt 6.5+ & GCC 11/12. | Pre-built `.rpm`, `.AppImage`, source build |
| **CentOS Stream 9** | **9.0+** (Full support) | 2.34 | **Tier-1 Native**: Direct `dnf` support without third-party repos. | Pre-built `.rpm`, `.AppImage`, source build |
| **RHEL / Rocky / AlmaLinux 8** | **8.6+** | 2.28 | **Supported via EPEL**: Requires EPEL 8 for Qt 6 and `gcc-toolset-11`. | Native source build (`make package-rpm`) |
| **CentOS 7 / RHEL 7** | *Unsupported* | 2.17 | **Incompatible / EOL**: Lacks Qt 6 and Modern C++17 compilers. | N/A (Migrate to RHEL/Rocky 9) |
| **SLES 15** | **15 SP4+** | 2.31 | **Supported**: Requires SUSE Package Hub for `qt6-base-devel`. | Source build, `.AppImage` (on SP6) |
| **openSUSE Leap** | **15.5+** / **Tumbleweed** | 2.38 | **Tier-1 Native**: Standard zypper repositories. | Pre-built `.rpm`, `.AppImage`, source build |
| **Ubuntu** | **22.04 LTS (Jammy)** / **24.04 LTS (Noble)** | 2.35 / 2.38 | **Tier-1 Native**: Standard universe/main repositories. | Pre-built `.deb`, `.AppImage`, source build |
| **Debian** | **12 (Bookworm)** / **13 (Trixie)** | 2.36 / 2.38 | **Tier-1 Native**: Standard APT repositories. | Pre-built `.deb`, `.AppImage`, source build |
| **Fedora** | **38+** | &ge; 2.37 | **Tier-1 Native**: Standard dnf repositories. | Pre-built `.rpm`, `.AppImage`, source build |

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

## 3. Pre-built Packages (AppImage, DEB, RPM)

Pre-compiled distribution packages and standalone binaries for Linux (x86_64) are generated automatically by CI workflows on every release and pull request, available from the GitHub [Releases](https://github.com/zkalves/rmap/releases) page or CI workflow artifacts.

To ensure broad enterprise and desktop Linux compatibility across differing glibc baselines, CI produces packages using multi-tiered container environments:
- **AlmaLinux 9 Container (`glibc 2.34` baseline)**: Used to generate the `.rpm` and `.AppImage`. Compatible with **RHEL 9**, **Rocky 9**, **Alma 9**, **CentOS Stream 9**, **Fedora 36+**, **SLES 15 SP6**, **Ubuntu 22.04+**, and **Debian 12+**.
- **Ubuntu 22.04 LTS Runner (`glibc 2.35` baseline)**: Used to generate the `.deb`. Compatible with **Ubuntu 22.04 LTS**, **Ubuntu 24.04 LTS**, and **Debian 12 (Bookworm)**.

### Standalone AppImage (Portable)
The AppImage is a single self-contained executable that bundles all Qt6 platform plugins, runtime libraries, templates, themes, and translations without requiring root privileges or system package installations:

```bash
# Make executable
chmod +x rmap-*-x86_64.AppImage

# Launch interactive GUI
./rmap-*-x86_64.AppImage

# Run headlessly in CI or offscreen server
QT_QPA_PLATFORM=offscreen ./rmap-*-x86_64.AppImage --version
```

### Debian / Ubuntu (.deb)
Install the standard Debian package with automatic dependency resolution:

```bash
# Install with apt (automatically resolves Qt6 and Protobuf dependencies)
sudo apt install ./rmap_*_amd64.deb

# Or install using dpkg:
sudo dpkg -i rmap_*_amd64.deb
sudo apt-get install -f  # resolve any missing runtime dependencies

# Launches directly from application menu or terminal:
rmap --version
```

The Debian package installs the executable to `/usr/bin/rmap`, registers the desktop launcher in `/usr/share/applications/rmap.desktop`, installs icons in `/usr/share/icons/hicolor/`, and bundles all templates, examples, themes, and translations in `/usr/share/rmap/` and offline documentation in `/usr/share/doc/rmap/`.

### Fedora / RHEL 9 / Rocky 9 / openSUSE (.rpm)
Install the Enterprise Linux RPM package using `dnf` or `zypper`:

```bash
# Fedora / RHEL 9 / Rocky Linux 9 / AlmaLinux 9 / CentOS Stream 9
sudo dnf install ./rmap-*.x86_64.rpm

# openSUSE Leap 15.6 / SLES 15 SP6
sudo zypper install ./rmap-*.x86_64.rpm
```

> [!NOTE]
> **RHEL 8 / Rocky 8 / AlmaLinux 8 Compatibility**:
> RHEL 8 features `glibc 2.28` and ships Qt 5 by default. To run `rmap` on RHEL 8, build directly from source using the EPEL 8 and `gcc-toolset-11` instructions below, or run `make package-rpm` on your RHEL 8 host to create an `el8`-native RPM package.

---

## 4. Installing from Source

You can install the `rmap` executable built from source to a system path or custom user directory.

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

### Generating Packages Locally

You can generate Debian, RPM, and AppImage packages locally using CMake/CPack or top-level `Makefile` targets:

```bash
# Build Debian (.deb) package in build/packages/
make package-deb
# (or: cd build && cpack -G DEB)

# Build RPM (.rpm) package (requires rpm / rpmbuild)
make package-rpm
# (or: cd build && cpack -G RPM)

# Build standalone AppImage package
make package-appimage
# (or: ./script/build_appimage.sh)

# Build all package formats simultaneously
make package
```

---

## 5. Launching the Application

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

## 6. Verifying Installation with Bundled Examples

**rmap** packages a rich suite of reference register maps and self-contained simulation environments in `<prefix>/share/rmap/examples` (and bundled templates in `<prefix>/share/rmap/templates`).

Users can copy the `examples` folder to any workspace and run the test targets directly to verify their installation:

```bash
# Copy examples to a working directory
cp -r /usr/local/share/rmap/examples ~/my_rmap_examples
# (or for custom install prefix: cp -r ~/.local/share/rmap/examples ~/my_rmap_examples)

cd ~/my_rmap_examples

# Run full simulation and compilation across all 19 example environments
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

[Next: GUI User Guide &rarr;](gui-guide.md)
