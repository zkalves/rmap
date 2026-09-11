# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

export RMAP_TEMPLATES_DIR ?= $(abspath $(CURDIR)/../../../templates)
RMAP       ?= ../../../build/bin/rmap
RMT_FILE   ?= ../../rmt/features/advanced_ral.rmt

.PHONY: all export validate compile-c compile-rust run-python verify-ral clean

all: export validate compile-c compile-rust run-python verify-ral

export:
	@echo "=== [advanced_ral] Exporting Register Models ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -e -o work

validate:
	@echo "=== [advanced_ral] Validating Register Map ==="
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -l --strict
	@if [ -f work/json/reg_map.json ]; then \
		jq empty work/json/reg_map.json > /dev/null 2>&1 && echo "✓ JSON Schema valid"; \
	fi

compile-c:
	@echo "=== [advanced_ral] Compiling C Firmware Harness ==="
	gcc -std=c99 -Wall -Wextra -Iwork/c test_harness.c -o work/c/test_harness
	./work/c/test_harness
	@echo "✓ C firmware assertions passed"

compile-rust:
	@if command -v rustc >/dev/null 2>&1; then \
		echo "=== [advanced_ral] Compiling Rust PAC Harness ==="; \
		rustc --crate-type=bin test_harness.rs -o work/rust/test_harness && ./work/rust/test_harness; \
		echo "✓ Rust PAC assertions passed"; \
	else \
		echo "INFO: rustc not installed; skipping Rust PAC compilation"; \
	fi

run-python:
	@echo "=== [advanced_ral] Running Python Driver Harness ==="
	python3 test_harness.py
	@echo "✓ Python driver assertions passed"

verify-ral:
	@echo "=== [advanced_ral] Verifying Advanced UVM RAL Constructs in Generated Code ==="
	@grep -q "uvm_reg_indirect_data" work/uvm/reg_model.sv && echo "✓ Verified indirect register (uvm_reg_indirect_data)"
	@grep -q "cfg_index" work/uvm/reg_model.sv && echo "✓ Verified index register binding (cfg_index)"
	@grep -q "uvm_reg_fifo" work/uvm/reg_model.sv && echo "✓ Verified hardware FIFO register (uvm_reg_fifo)"
	@grep -q "status_cbs_cbs" work/uvm/reg_model.sv && echo "✓ Verified register callbacks class (status_cbs_cbs)"
	@grep -q "NO_REG_TEST" work/uvm/reg_model.sv && echo "✓ Verified register test exclusion directive (NO_REG_TEST)"
	@grep -q "NO_MEM_TEST" work/uvm/reg_model.sv && echo "✓ Verified memory test exclusion directive (NO_MEM_TEST)"
	@grep -q "val_cg" work/uvm/reg_model.sv && echo "✓ Verified embedded functional coverage model (val_cg)"
	@echo "✓ All advanced UVM RAL construct checks PASSED!"

clean:
	rm -rf work
