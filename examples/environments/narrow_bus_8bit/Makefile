# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

# Locate templates directory: check local repository or relative folders, otherwise let rmap locate installed templates
REPO_TEMPLATES ?= $(firstword $(wildcard $(CURDIR)/../../../templates $(CURDIR)/../../templates $(CURDIR)/../templates))
ifneq ($(REPO_TEMPLATES),)
  export RMAP_TEMPLATES_DIR ?= $(abspath $(REPO_TEMPLATES))
endif

# Locate rmap executable: check local build tree first, then search system PATH
RMAP       ?= $(shell if [ -x "$(CURDIR)/../../../build/bin/rmap" ]; then echo "$(CURDIR)/../../../build/bin/rmap"; elif command -v rmap >/dev/null 2>&1; then command -v rmap; else echo "rmap"; fi)
RMT_FILE   ?= ../../rmt/features/narrow_bus_8bit.rmt

.PHONY: all export validate compile-c run-python check-width clean

all: export validate check-width compile-c run-python

export:
	@echo "=== [narrow_bus_8bit] Exporting Register Models ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -e -o work

validate:
	@echo "=== [narrow_bus_8bit] Validating Register Map ==="
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -l
	@if [ -f work/json/reg_map.json ]; then \
		jq empty work/json/reg_map.json > /dev/null 2>&1 && echo "✓ JSON Schema valid"; \
	fi

check-width:
	@echo "=== [narrow_bus_8bit] Verifying 8-bit Data Bus Width Architecture ==="
	@grep -q "DATA_WIDTH = 8" work/rtl/reg_map.sv && echo "✓ SystemVerilog has DATA_WIDTH = 8"
	@grep -q "DATA_WIDTH = 8" work/rtl/reg_map.v && echo "✓ Verilog-2001 has DATA_WIDTH = 8"
	@grep -q "DATA_WIDTH : integer := 8" work/rtl/reg_map.vhd && echo "✓ VHDL has DATA_WIDTH = 8"
	@grep -q "DATA_WIDTH = 8" work/rtl/apb_reg_file.sv && echo "✓ APB adapter has DATA_WIDTH = 8"
	@grep -q "DATA_WIDTH = 8" work/rtl/axil_reg_file.sv && echo "✓ AXI4-Lite adapter has DATA_WIDTH = 8"
	@grep -q "DATA_WIDTH = 8" work/rtl/reg_map_sva.sv && echo "✓ Formal SVA has DATA_WIDTH = 8"

compile-c:
	@echo "=== [narrow_bus_8bit] Compiling C Firmware Harness ==="
	gcc -std=c99 -Wall -Wextra -Iwork/c test_harness.c -o work/c/test_harness
	./work/c/test_harness
	@echo "✓ C firmware assertions passed"

run-python:
	@echo "=== [narrow_bus_8bit] Running Python Driver Harness ==="
	python3 test_harness.py
	@echo "✓ Python driver assertions passed"

clean:
	rm -rf work
