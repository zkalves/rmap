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
RMT_FILE   ?= decode_only.rmt

.PHONY: all export validate check-signals clean

all: export validate check-signals

export:
	@echo "=== [decode_only] Exporting Register Models ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -e -o work

validate:
	@echo "=== [decode_only] Validating Register Map ==="
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(RMT_FILE) -l
	@if [ -f work/json/reg_map.json ]; then \
		jq empty work/json/reg_map.json > /dev/null 2>&1 && echo "✓ JSON Schema valid"; \
	fi

check-signals:
	@echo "=== [decode_only] Verifying Decode-Only Signals in RTL ==="
	@grep -q "sw_reg_decode_only_wr_valid_o" work/rtl/reg_map.sv && echo "✓ SystemVerilog has sw_reg_decode_only_wr_valid_o"
	@grep -q "sw_reg_decode_only_rd_valid_o" work/rtl/reg_map.sv && echo "✓ SystemVerilog has sw_reg_decode_only_rd_valid_o"
	@grep -q "sw_reg_decode_only_ready_i" work/rtl/reg_map.sv && echo "✓ SystemVerilog has sw_reg_decode_only_ready_i"
	@grep -q "sw_reg_decode_only_error_i" work/rtl/reg_map.sv && echo "✓ SystemVerilog has sw_reg_decode_only_error_i"
	@grep -q "sw_reg_mixed_ext_val_wr_valid_o" work/rtl/reg_map.sv && echo "✓ SystemVerilog has sw_reg_mixed_ext_val_wr_valid_o"
	@grep -q "hw_reg_mixed_ext_val_i" work/rtl/reg_map.sv && echo "✓ SystemVerilog has hw_reg_mixed_ext_val_i"
	@! grep -q "sw_reg_mixed_wr_valid_o" work/rtl/reg_map.sv && echo "✓ SystemVerilog correctly omits register-level valid for mixed register"
	@! grep -q "sw_reg_mixed_int_val_wr_valid_o" work/rtl/reg_map.sv && echo "✓ SystemVerilog correctly omits field-level valid for normal field"
	@! grep -q "reg_reg_decode_only_q" work/rtl/reg_map.sv && echo "✓ SystemVerilog correctly omits storage flip-flop for decode-only register"
	@grep -q "reg_reg_mixed_q" work/rtl/reg_map.sv && echo "✓ SystemVerilog preserves storage flip-flop for normal field in mixed register"
	@grep -q "sw_reg_decode_only_wr_valid_o" work/rtl/reg_map.v && echo "✓ Verilog-2001 has sw_reg_decode_only_wr_valid_o"
	@grep -q "sw_reg_decode_only_wr_valid_o" work/rtl/reg_map.vhd && echo "✓ VHDL has sw_reg_decode_only_wr_valid_o"
	@grep -q "sw_reg_decode_only_wr_valid_o" work/rtl/apb_reg_file.sv && echo "✓ APB4 wrapper connects sw_reg_decode_only_wr_valid_o"
	@grep -q "sw_reg_decode_only_wr_valid_o" work/rtl/axil_reg_file.sv && echo "✓ AXI4-Lite wrapper connects sw_reg_decode_only_wr_valid_o"

clean:
	rm -rf work
