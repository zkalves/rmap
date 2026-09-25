# Getting Started {#getting_started}

Welcome to **rmap**! This guide helps you get started quickly with **rmap**, whether you prefer pre-compiled distribution packages (zero build tools required) or compiling from source.

---

## Choosing Your Installation Method

| Installation Method | Best For | Prerequisites Required | Installation Effort |
| :--- | :--- | :--- | :--- |
| <b>Option 1: Standalone AppImage (Recommended)</b> | Quick evaluation, portable across all Linux distros | None (fully self-contained) | Download & run (`chmod +x`) |
| <b>Option 2: Native Package (.deb / .rpm)</b> | Desktop users, standard workstation installations | System package manager (`apt` or `dnf`) | 1 command (`apt install` / `dnf install`) |
| <b>Option 3: OCI Container Image</b> | CI/CD pipelines, containerized build environments | Docker or Podman | 1 command (`docker run`) |
| <b>Option 4: Homebrew Formula</b> | macOS users and Linuxbrew environments | Homebrew (`brew`) | 1 command (`brew install`) |
| <b>Option 5: Environment Modules / Lmod</b> | Shared EDA server clusters & HPC environments | `lmod` or `environment-modules` | Extract & `module load` |
| <b>Option 6: Compiling from Source</b> | Developers, contributors, custom deployments | C++17 compiler, CMake, Qt 6 & Protobuf dev headers | Build with `make` or `cmake` |

---

## 1. Quick Start: Pre-built Packages (No Build Tools Required)

Pre-built distribution packages and standalone executables for 64-bit Linux (`x86_64`) are published on the GitHub [Releases](https://github.com/zkalves/rmap/releases) page.

### Option A: Standalone AppImage (All Linux Distributions)

The AppImage is a single self-contained executable that bundles all Qt6 platform plugins, runtime libraries, templates, themes, and translations. It requires no installation, no root privileges, and no external dependencies:

```bash
# Download and make executable
chmod +x rmap-*-x86_64.AppImage

# Launch interactive GUI
./rmap-*-x86_64.AppImage

# Or execute headlessly in CI or offscreen server
QT_QPA_PLATFORM=offscreen ./rmap-*-x86_64.AppImage --version
```

### Option B: Debian / Ubuntu Package (`.deb`)

Installs `rmap` system-wide and registers desktop shortcuts, icons, and MIME types. APT automatically resolves the necessary runtime libraries:

```bash
# Install with apt (automatically resolves runtime dependencies)
sudo apt update
sudo apt install ./rmap_*_amd64.deb

# Verify installation
rmap --version
```

### Option C: Fedora / RHEL 9 / Rocky Linux / openSUSE (`.rpm`)

Installs `rmap` system-wide using your distribution's native package manager:

```bash
# Fedora / RHEL 9 / Rocky Linux 9 / AlmaLinux 9 / CentOS Stream 9
sudo dnf install ./rmap-*.x86_64.rpm

# openSUSE Leap 15.6 / SLES 15 SP6
sudo zypper install ./rmap-*.x86_64.rpm

# Verify installation
rmap --version
```

### Option D: OCI / Docker Container Image (Docker / Podman)

Official container images are published to the GitHub Container Registry (`ghcr.io/zkalves/rmap:latest`). The container bundles all dependencies and templates, providing a clean, reproducible headless execution environment:

```bash
# Pull the latest container image
docker pull ghcr.io/zkalves/rmap:latest

# Display version
docker run --rm ghcr.io/zkalves/rmap:latest rmap --version

# Run headless code generation mounting the current directory
docker run --rm -v "$(pwd):/work" -w /work ghcr.io/zkalves/rmap:latest \
  rmap -f examples/rmt/peripherals/spi.rmt --export --out ./work

# Podman (rootless container runtime)
podman run --rm -v "$(pwd):/work:Z" -w /work ghcr.io/zkalves/rmap:latest \
  rmap --help
```

### Option E: Homebrew Formula (macOS & Linux)

Install `rmap` using Homebrew on macOS (Apple Silicon and Intel) or Linux (Linuxbrew):

```bash
# Tap repository and install rmap
brew install zkalves/rmap/rmap

# Or install from local formula in repository
brew install --build-from-source Formula/rmap.rb

# Verify installation
rmap --version
```

### Option F: Environment Modules / Lmod (HPC & EDA Clusters)

For semiconductor design teams, shared EDA compute clusters, and HPC environments utilizing Lmod or classical Environment Modules, `rmap` provides relocatable module packages:

```bash
# Extract relocatable package into cluster software repository (e.g., /tools)
tar -xzf rmap-0.2.0-module-linux-x86_64.tar.gz -C /tools/

# Add modulefiles directory to MODULEPATH
module use /tools/modulefiles

# Load rmap module into current shell session
module load rmap/0.2.0

# Verify environment variables (PATH, RMAP_DIR, RMAP_TEMPLATE_PATH)
rmap --version
```

> [!TIP]
> For the complete distribution compatibility matrix, glibc baselines, and packaging details, see the [Supported Operating Systems & Compatibility Matrix Guide](@ref supported_os).

---

## 2. Compiling from Source

Follow these steps **only** if you wish to build **rmap** from source code, customize functionality, or contribute to development.

### Build Toolchain & Development Dependencies
- **C++ Compiler**: Modern C++17 compiler (`g++` 9+ or `clang++` 10+)
- **Build System**: CMake 3.15+ and `make`
- **Qt 6 Development Packages**: `QtCore`, `QtWidgets`, `QtTest` (`qt6-base-dev` / `qt6-qtbase-devel`)
- **Google Protocol Buffers**: `protobuf-compiler` and `libprotobuf-dev` / `protobuf-devel`

### Installing Build Dependencies by Distribution

#### Ubuntu / Debian (22.04 LTS / 24.04 LTS / Debian 12+)
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
RHEL 8 features older baseline packages and requires **EPEL 8** for Qt 6 and **GCC Toolset 11** for Modern C++17:
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

### Building rmap with Make

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

After building from source, install the `rmap` binary and resource files to a system path or user directory.

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

[Next: Supported Operating Systems &rarr;](@ref supported_os) | [GUI User Guide &rarr;](@ref gui_guide)
