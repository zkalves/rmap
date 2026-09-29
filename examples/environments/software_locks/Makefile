# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

# Locate templates directory: check local repository or relative folders
REPO_TEMPLATES ?= $(firstword $(wildcard $(CURDIR)/../../../templates $(CURDIR)/../../templates $(CURDIR)/../templates))
ifneq ($(REPO_TEMPLATES),)
  export RMAP_TEMPLATES_DIR ?= $(abspath $(REPO_TEMPLATES))
endif

# Locate rmap executable: check local build tree first, then search system PATH
RMAP       ?= $(shell if [ -x "$(CURDIR)/../../../build/bin/rmap" ]; then echo "$(CURDIR)/../../../build/bin/rmap"; elif command -v rmap >/dev/null 2>&1; then command -v rmap; else echo "rmap"; fi)
RMT_FILE   ?= ../../rmt/features/software_locks.rmt

.PHONY: all export validate check-locks compile-c run-python clean

all: export validate check-locks compile-c run-python

export:
	@echo "=== [software_locks] Exporting Register Models ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -e -o work

validate:
	@echo "=== [software_locks] Validating Register Map ==="
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -l
	@if [ -f work/json/reg_map.json ]; then \
		jq empty work/json/reg_map.json > /dev/null 2>&1 && echo "✓ JSON Schema valid"; \
	fi

check-locks:
	@echo "=== [software_locks] Verifying Hierarchical Software Lock Signals in RTL ==="
	@grep -q "blk_wr_locked" work/rtl/reg_map.sv && echo "✓ SystemVerilog has blk_wr_locked"
	@grep -q "blk_rd_locked" work/rtl/reg_map.sv && echo "✓ SystemVerilog has blk_rd_locked"
	@grep -q "reg_sec_key_wr_locked" work/rtl/reg_map.sv && echo "✓ SystemVerilog has reg_sec_key_wr_locked"
	@grep -q "reg_multi_field_reg_key_slice_wr_locked" work/rtl/reg_map.sv && echo "✓ SystemVerilog has field-level write lock"
	@grep -q "reg_multi_field_reg_passcode_rd_locked" work/rtl/reg_map.sv && echo "✓ SystemVerilog has field-level read lock"
	@grep -q "blk_wr_locked" work/rtl/reg_map.v && echo "✓ Verilog-2001 has blk_wr_locked"
	@grep -q "blk_wr_locked" work/rtl/reg_map.vhd && echo "✓ VHDL has blk_wr_locked"
	@grep -q "assert_lock_sec_key_immutable" work/rtl/reg_map_sva.sv && echo "✓ Formal SVA has assert_lock_sec_key_immutable"
	@grep -q "assert_lock_sec_key_rd_zero" work/rtl/reg_map_sva.sv && echo "✓ Formal SVA has assert_lock_sec_key_rd_zero"
	@grep -q "assert_lock_multi_field_reg_key_slice_immutable" work/rtl/reg_map_sva.sv && echo "✓ Formal SVA has field-level immutability assertion"
	@grep -q "assert_lock_multi_field_reg_passcode_rd_zero" work/rtl/reg_map_sva.sv && echo "✓ Formal SVA has field-level read-zero assertion"

compile-c:
	@echo "=== [software_locks] Compiling C Firmware Harness ==="
	gcc -std=c99 -Wall -Wextra -Iwork/c test_harness.c -o work/c/test_harness
	./work/c/test_harness
	@echo "✓ C firmware assertions passed"

run-python:
	@echo "=== [software_locks] Running Python Driver Harness ==="
	python3 test_harness.py
	@echo "✓ Python driver assertions passed"

clean:
	rm -rf work
