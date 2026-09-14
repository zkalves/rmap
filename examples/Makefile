# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

export RMAP_TEMPLATES_DIR ?= $(abspath $(CURDIR)/../templates)

ENVIRONMENTS := $(wildcard environments/*)

.PHONY: all clean $(ENVIRONMENTS)

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

clean:
	@for env in $(ENVIRONMENTS); do \
		if [ -d "$$env" ]; then \
			$(MAKE) -C "$$env" clean; \
		fi; \
	done
