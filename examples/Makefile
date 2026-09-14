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

export RMAP_TEMPLATES_DIR ?= $(abspath $(CURDIR)/../templates)

ENVIRONMENTS := $(wildcard environments/*)

.PHONY: all clean sim-uvm $(ENVIRONMENTS)

all:
	@echo "=== Running Autonomous Simulation & Compilation Across All Examples ==="
	@for env in $(ENVIRONMENTS); do \
		if [ -d "$$env" ]; then \
			echo ""; \
			echo "----------------------------------------------------------------"; \
			echo "==> Running Example Environment: $$env"; \
			echo "----------------------------------------------------------------"; \
			$(MAKE) -C "$$env" all || exit 1; \
			$(MAKE) -C "$$env" clean || exit 1; \
		fi; \
	done
	@echo ""
	@echo "================================================================"
	@echo "✓ All $(words $(ENVIRONMENTS)) example environments PASSED successfully!"
	@echo "================================================================"

sim-uvm:
	@echo "=== Running UVM Verification Across Examples ==="
	@for env in $(ENVIRONMENTS); do \
		if [ -d "$$env" ]; then \
			$(MAKE) -C "$$env" sim-uvm SIM=$(SIM) UVM_VER=$(UVM_VER) UVM_HOME=$(UVM_HOME) || exit 1; \
		fi; \
	done

clean:
	@for env in $(ENVIRONMENTS); do \
		if [ -d "$$env" ]; then \
			$(MAKE) -C "$$env" clean; \
		fi; \
	done

