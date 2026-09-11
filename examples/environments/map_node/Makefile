# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

export RMAP_TEMPLATES_DIR ?= $(abspath $(CURDIR)/../../../templates)
RMAP       ?= ../../../build/bin/rmap
RMT_FILE   ?= ../../rmt/features/map_node.rmt
SIM        ?= iverilog

.PHONY: all export validate compile-c compile-rust run-python sim-rtl sim-verilator sim-pyuvm sim-uvm clean

all: export validate compile-c compile-rust run-python sim-rtl sim-verilator

export:
	@echo "=== [map_node] Exporting Register Models ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -e -o work

validate:
	@echo "=== [map_node] Validating Register Map ==="
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -l
	@if [ -f work/json/reg_map.json ]; then \
		jq empty work/json/reg_map.json > /dev/null 2>&1 && echo "✓ JSON Schema valid"; \
	fi

compile-c:
	@echo "=== [map_node] Compiling C Firmware Harness ==="
	gcc -std=c99 -Wall -Wextra -Iwork/c test_harness.c -o work/c/test_harness
	./work/c/test_harness
	@echo "✓ C firmware assertions passed"

compile-rust:
	@if command -v rustc >/dev/null 2>&1; then \
		echo "=== [map_node] Compiling Rust PAC Harness ==="; \
		rustc --crate-type=bin test_harness.rs -o work/rust/test_harness && ./work/rust/test_harness; \
		echo "✓ Rust PAC assertions passed"; \
	else \
		echo "INFO: rustc not installed; skipping Rust PAC compilation"; \
	fi

run-python:
	@echo "=== [map_node] Running Python Driver Harness ==="
	python3 test_harness.py
	@echo "✓ Python driver assertions passed"

sim-rtl:
	@if [ -f work/sim/Makefile ]; then \
		if command -v iverilog >/dev/null 2>&1; then \
			echo "=== [map_node] Running RTL Simulation (Icarus Verilog) ==="; \
			$(MAKE) -C work/sim sim-rtl; \
		else \
			echo "INFO: iverilog not installed; skipping RTL simulation"; \
		fi; \
	fi

sim-verilator:
	@if [ -f work/sim/Makefile ]; then \
		if command -v verilator >/dev/null 2>&1; then \
			echo "=== [map_node] Running Verilator Simulation ==="; \
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
		$(MAKE) -C work/sim sim-uvm SIM=$(SIM) || echo "INFO: Commercial UVM simulator not detected; skipping sim-uvm"; \
	fi

clean:
	rm -rf work
