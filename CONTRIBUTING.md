# Contributing to rmap

Thank you for your interest in contributing to **rmap**! Whether you are reporting a bug, improving documentation, adding support for a new register format, or enhancing code generation templates, we welcome your contributions.

---

## Code of Conduct

We are committed to providing a welcoming, inclusive, and harassment-free environment for everyone. Please treat all contributors and maintainers with respect, empathy, and professional courtesy.

---

## How Can I Contribute?

### Reporting Bugs
Before submitting a new issue, please search existing issues to see if it has already been reported.

If submitting a new bug report, please include:
- **Environment**: OS version, Qt version (`qmake6 --version` or `qtpaths6 --version`), and compiler (`g++ --version` / `clang++ --version`).
- **Steps to reproduce**: Clear, minimal steps to trigger the bug.
- **Sample input**: If applicable, a minimal register map file (`.rmt`, `.rdl`, `.svd`, etc.) reproducing the issue.
- **Expected vs. Actual behavior**: What you expected to happen vs. what actually occurred.

### Proposing Enhancements
Feature requests are welcome! Please open an issue outlining:
- The problem or workflow limitation you are trying to solve.
- Proposed solution and design considerations.
- If proposing new code generation templates, sample output files or target verification environments.

---

## Development Workflow

### 1. Prerequisites
Ensure you have the required build tools and libraries installed:

**Ubuntu / Debian**:
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git \
  qt6-base-dev libqt6test6 \
  libprotobuf-dev protobuf-compiler
```

**Fedora / RHEL**:
```bash
sudo dnf install -y gcc-c++ cmake git \
  qt6-qtbase-devel \
  protobuf-devel protobuf-compiler
```

### 2. Building from Source
```bash
# Clone repository
git clone https://github.com/zkalves/rmap.git
cd rmap

# Build all targets and tests using Makefile wrapper
make

# Or build only application binary
make rmap

# Install to default location (/usr/local/bin)
sudo make install

# Install to a custom directory (e.g. user home directory)
make install PREFIX=$HOME/.local

# Or configure with CMake custom prefix
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build -j$(nproc)
cmake --install build
```

### 3. Running Automated Tests
Every pull request must maintain a **100% test pass rate**:

```bash
# Run all C++ unit tests headlessly (backend + frontend)
make test-unit

# Run template verification tests across all output targets
make test-templates

# Run complete verification (unit tests + templates)
make test-all
```

---

## Code Guidelines

### Modern C++ & Qt Standards
- **C++ Standard**: Modern C++17 (`std::string_view`, structured bindings, `std::optional`, `std::filesystem`).
- **Qt 6 Framework**: Idiomatic Qt (`QObject` parent-child memory ownership, Qt model/view architecture, signals/slots).
- **Zero Raw Pointers for Ownership**: Prefer `std::unique_ptr`, `std::shared_ptr`, or Qt parent-child hierarchy.
- **Const Correctness**: Mark methods and read-only parameters `const`.

### License Headers
All new C++, Python, Shell, and configuration source files must include the Mozilla Public License 2.0 banner at the top:

```cpp
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */
```

### Testing Policy
- **Regression Prevention**: Any bug fix must include a reproducing test case in `tests/backend/` or `tests/frontend/`.
- **New Features**: New parsers, widgets, or generators must be accompanied by unit tests covering nominal and edge cases.
- **Deterministic Isolation**: Test suites must clean up temporary directories (`work/`) in `initTestCase()` and `cleanupTestCase()`.

---

## Submitting a Pull Request

1. **Fork** the repository and create a descriptive branch name:
   ```bash
   git checkout -b feat/my-new-feature
   # or
   git checkout -b fix/issue-description
   ```
2. Make changes and verify that all tests pass (`make test-all`).
3. Commit with clear, conventional messages:
   ```bash
   git commit -m "[feat] Add support for APB4 bus wrapper in RTL generation"
   git commit -m "[fix] Correct bitmask calculation for 64-bit wide registers"
   ```
4. Push to your fork and open a Pull Request against `main`.
5. Ensure GitHub Actions CI passes all matrix jobs.
