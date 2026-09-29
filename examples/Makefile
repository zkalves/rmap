# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

# Locate rmap executable: check local build tree first, then search system PATH
RMAP ?= $(shell if [ -x "$(CURDIR)/../build/bin/rmap" ]; then echo "$(CURDIR)/../build/bin/rmap"; elif command -v rmap >/dev/null 2>&1; then command -v rmap; else echo "rmap"; fi)
export RMAP

# Locate templates directory: check local repository or relative folders, otherwise let rmap locate installed templates
REPO_TEMPLATES ?= $(firstword $(wildcard $(CURDIR)/../templates $(CURDIR)/templates))
ifneq ($(REPO_TEMPLATES),)
  export RMAP_TEMPLATES_DIR ?= $(abspath $(REPO_TEMPLATES))
endif

EXAMPLES := $(sort $(wildcard peripherals/*) $(wildcard features/*))

.PHONY: all clean sim-uvm $(EXAMPLES)

all:
	@echo "=== Running Autonomous Simulation & Compilation Across All Examples ==="
	@for ex in $(EXAMPLES); do \
		if [ -d "$$ex" ]; then \
			echo ""; \
			echo "----------------------------------------------------------------"; \
			echo "==> Running Example: $$ex"; \
			echo "----------------------------------------------------------------"; \
			$(MAKE) -C "$$ex" all || exit 1; \
			$(MAKE) -C "$$ex" clean || exit 1; \
		fi; \
	done
	@echo ""
	@echo "================================================================"
	@echo "✓ All $(words $(EXAMPLES)) examples PASSED successfully!"
	@echo "================================================================"

sim-uvm:
	@echo "=== Running UVM Verification Across Examples ==="
	@for ex in $(EXAMPLES); do \
		if [ -d "$$ex" ]; then \
			$(MAKE) -C "$$ex" sim-uvm SIM=$(SIM) UVM_VER=$(UVM_VER) UVM_HOME=$(UVM_HOME) || exit 1; \
		fi; \
	done

clean:
	@for ex in $(EXAMPLES); do \
		if [ -d "$$ex" ]; then \
			$(MAKE) -C "$$ex" clean; \
		fi; \
	done

