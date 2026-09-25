# Supported Operating Systems & Compatibility Matrix {#supported_os}

This document defines the supported operating systems, Linux distribution compatibility baselines, glibc requirements, and pre-built package deliverables for **rmap**.

---

## 1. Supported Linux Distributions & Compatibility Matrix

**rmap** supports modern enterprise and desktop Linux distributions on 64-bit (`x86_64`) architectures:

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

### macOS Support
- **macOS 12+ (Monterey, Ventura, Sonoma, Sequoia)**: Supported for source builds via Homebrew (`brew install cmake qt@6 protobuf`). Apple Silicon (M1/M2/M3/M4) and Intel x86_64 architectures are supported natively.

### Windows Support
- **Windows 10 / 11 via WSL2 (Ubuntu 22.04 / 24.04)**: Supported natively with full GUI support via WSLg or headless offscreen CLI execution.

### Pre-built Distribution Packages

Pre-compiled distribution packages and standalone binaries for Linux (x86_64) are generated automatically by CI workflows on every release and pull request, available from the GitHub [Releases](https://github.com/zkalves/rmap/releases) page or CI workflow artifacts.

To ensure broad enterprise and desktop Linux compatibility across differing glibc baselines, CI produces packages using multi-tiered container environments:
- **AlmaLinux 9 Container (`glibc 2.34` baseline)**: Used to generate the `.rpm` and `.AppImage`. Compatible with **RHEL 9**, **Rocky 9**, **Alma 9**, **CentOS Stream 9**, **Fedora 36+**, **SLES 15 SP6**, **Ubuntu 22.04+**, and **Debian 12+**.
- **Ubuntu 22.04 LTS Runner (`glibc 2.35` baseline)**: Used to generate the `.deb`. Compatible with **Ubuntu 22.04 LTS**, **Ubuntu 24.04 LTS**, and **Debian 12 (Bookworm)**.

#### Standalone AppImage (Portable)
The AppImage is a single self-contained executable that bundles all Qt6 platform plugins, runtime libraries, templates, themes, and translations without requiring root privileges or system package installations:

```bash
# Make executable
chmod +x rmap-*-x86_64.AppImage

# Launch interactive GUI
./rmap-*-x86_64.AppImage

# Run headlessly in CI or offscreen server
QT_QPA_PLATFORM=offscreen ./rmap-*-x86_64.AppImage --version
```

#### Debian / Ubuntu (.deb)
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

The Debian package installs the executable to `/usr/bin/rmap`, registers the desktop launcher in `/usr/share/applications/rmap.desktop`, installs icons in `/usr/share/icons/hicolor/`, bundles all templates, examples, themes, and translations in `/usr/share/rmap/`, and installs offline documentation in `/usr/share/doc/rmap/`.

#### Fedora / RHEL 9 / Rocky 9 / openSUSE (.rpm)
Install the Enterprise Linux RPM package using `dnf` or `zypper`:

```bash
# Fedora / RHEL 9 / Rocky Linux 9 / AlmaLinux 9 / CentOS Stream 9
sudo dnf install ./rmap-*.x86_64.rpm

# openSUSE Leap 15.6 / SLES 15 SP6
sudo zypper install ./rmap-*.x86_64.rpm
```

> [!NOTE]
> **RHEL 8 / Rocky 8 / AlmaLinux 8 Compatibility**:
> RHEL 8 features `glibc 2.28` and ships Qt 5 by default. To run `rmap` on RHEL 8, build directly from source using the EPEL 8 and `gcc-toolset-11` instructions, or run `make package-rpm` on your RHEL 8 host to create an `el8`-native RPM package.

---

## 2. Generating Packages Locally

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

[Back to Getting Started](@ref getting_started) | [Next: GUI User Guide &rarr;](@ref gui_guide)
