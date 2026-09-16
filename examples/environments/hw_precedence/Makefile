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
RMT_FILE   ?= ../../rmt/features/hw_precedence.rmt

.PHONY: all export validate compile-c run-python check-precedence clean

all: export validate check-precedence compile-c run-python

export:
	@echo "=== [hw_precedence] Exporting Register Models ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -e -o work

validate:
	@echo "=== [hw_precedence] Validating Register Map ==="
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -l
	@if [ -f work/json/reg_map.json ]; then \
		jq empty work/json/reg_map.json > /dev/null 2>&1 && echo "✓ JSON Schema valid"; \
	fi

check-precedence:
	@echo "=== [hw_precedence] Verifying Software Arbitration Precedence (PARAM_HW_PRECEDENCE = 0) ==="
	@grep -q "PARAM_HW_PRECEDENCE = 0" work/rtl/reg_map.sv && echo "✓ SystemVerilog has PARAM_HW_PRECEDENCE = 0"
	@grep -q "PARAM_HW_PRECEDENCE = 0" work/rtl/reg_map.v && echo "✓ Verilog-2001 has PARAM_HW_PRECEDENCE = 0"
	@grep -q "PARAM_HW_PRECEDENCE : std_logic := '0'" work/rtl/reg_map.vhd && echo "✓ VHDL has PARAM_HW_PRECEDENCE = '0'"
	@grep -q "PARAM_HW_PRECEDENCE = 0" work/rtl/apb_reg_file.sv && echo "✓ APB adapter has PARAM_HW_PRECEDENCE = 0"
	@grep -q "PARAM_HW_PRECEDENCE = 0" work/rtl/axil_reg_file.sv && echo "✓ AXI4-Lite adapter has PARAM_HW_PRECEDENCE = 0"
	@grep -q "PARAM_HW_PRECEDENCE = 0" work/rtl/reg_map_sva.sv && echo "✓ Formal SVA has PARAM_HW_PRECEDENCE = 0"

compile-c:
	@echo "=== [hw_precedence] Compiling C Firmware Harness ==="
	gcc -std=c99 -Wall -Wextra -Iwork/c test_harness.c -o work/c/test_harness
	./work/c/test_harness
	@echo "✓ C firmware assertions passed"

run-python:
	@echo "=== [hw_precedence] Running Python Driver Harness ==="
	python3 test_harness.py
	@echo "✓ Python driver assertions passed"

clean:
	rm -rf work
