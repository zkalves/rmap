# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

RMAP      ?= ../../../build/bin/rmap
FILE_V1   ?= ../../rmt/features/diff_v1.rmt
FILE_V2   ?= ../../rmt/features/diff_v2.rmt

.PHONY: all export diff diff-md validate clean

all: export validate diff diff-md

export:
	@echo "=== [diff] Exporting V1 & V2 Register Models ==="
	@mkdir -p work/v1 work/v2
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(FILE_V1) -e -o work/v1
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(FILE_V2) -e -o work/v2

validate:
	@echo "=== [diff] Validating V1 & V2 Register Maps ==="
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(FILE_V1) -l
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(FILE_V2) -l

diff:
	@echo "=== [diff] Performing Semantic Register Map Comparison (Text) ==="
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(FILE_V1) -d $(FILE_V2)

diff-md:
	@echo "=== [diff] Generating Markdown Diff Report ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f $(FILE_V1) -d $(FILE_V2) --report-format markdown -o work/diff_report.md
	@if [ -f work/diff_report.md ]; then \
		echo "✓ Markdown diff report generated:"; \
		cat work/diff_report.md; \
	fi

clean:
	rm -rf work
