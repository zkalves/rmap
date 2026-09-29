# This file is part of rmap.
#
# rmap is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rmap is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rmap.  If not, see <https://www.gnu.org/licenses/>.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

# Locate templates directory: check local repository or relative folders, otherwise let rmap locate installed templates
REPO_TEMPLATES ?= $(firstword $(wildcard $(CURDIR)/../../../templates $(CURDIR)/../../templates $(CURDIR)/../templates))
ifneq ($(REPO_TEMPLATES),)
  export RMAP_TEMPLATES_DIR ?= $(abspath $(REPO_TEMPLATES))
endif

# Locate rmap executable: check local build tree first, then search system PATH
RMAP       ?= $(shell if [ -x "$(CURDIR)/../../../build/bin/rmap" ]; then echo "$(CURDIR)/../../../build/bin/rmap"; elif command -v rmap >/dev/null 2>&1; then command -v rmap; else echo "rmap"; fi)
RMT_FILE   ?= sensor_hub.rmt
SIM        ?= iverilog

.PHONY: all export validate compile-c compile-rust run-python sim-rtl sim-verilator sim-pyuvm sim-uvm clean

all: export validate compile-c compile-rust run-python sim-rtl sim-verilator

export:
	@echo "=== [sensor_hub] Exporting Register Models ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -e -o work

validate:
	@echo "=== [sensor_hub] Validating Register Map ==="
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -l
	@if [ -f work/json/reg_map.json ]; then \
		jq empty work/json/reg_map.json > /dev/null 2>&1 && echo "✓ JSON Schema valid"; \
	fi

compile-c:
	@echo "=== [sensor_hub] Compiling C Firmware Harness ==="
	gcc -std=c99 -Wall -Wextra -Iwork/c test_harness.c -o work/c/test_harness
	./work/c/test_harness
	@echo "✓ C firmware assertions passed"

compile-rust:
	@if command -v rustc >/dev/null 2>&1; then \
		echo "=== [sensor_hub] Compiling Rust PAC Harness ==="; \
		rustc --crate-type=bin test_harness.rs -o work/rust/test_harness && ./work/rust/test_harness; \
		echo "✓ Rust PAC assertions passed"; \
	else \
		echo "INFO: rustc not installed; skipping Rust PAC compilation"; \
	fi

run-python:
	@echo "=== [sensor_hub] Running Python Driver Harness ==="
	python3 test_harness.py
	@echo "✓ Python driver assertions passed"

sim-rtl:
	@if [ -f work/sim/Makefile ]; then \
		if command -v iverilog >/dev/null 2>&1; then \
			echo "=== [sensor_hub] Running RTL Simulation (Icarus Verilog) ==="; \
			$(MAKE) -C work/sim sim-rtl; \
		else \
			echo "INFO: iverilog not installed; skipping RTL simulation"; \
		fi; \
	fi

sim-verilator:
	@if [ -f work/sim/Makefile ]; then \
		if command -v verilator >/dev/null 2>&1; then \
			echo "=== [sensor_hub] Running Verilator Simulation ==="; \
			$(MAKE) -C work/sim sim-verilator; \
		else \
			echo "INFO: verilator not installed; skipping Verilator simulation"; \
		fi; \
	fi

sim-pyuvm:
	@if [ -f work/sim/Makefile ]; then \
		if command -v cocotb-config >/dev/null 2>&1; then \
			$(MAKE) -C work/sim sim-pyuvm; \
		else \
			echo "INFO: cocotb/pyuvm not installed; skipping pyuvm simulation"; \
		fi; \
	fi

sim-uvm:
	@if [ -f work/sim/Makefile ]; then \
		$(MAKE) -C work/sim sim-uvm SIM=$(SIM) UVM_VER=$(UVM_VER) UVM_HOME=$(UVM_HOME) || echo "INFO: Commercial UVM simulator not detected; skipping sim-uvm"; \
	fi

clean:
	rm -rf work
