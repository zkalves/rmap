# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

export RMAP_TEMPLATES_DIR ?= $(abspath $(CURDIR)/../../../templates)
RMAP       ?= ../../../build/bin/rmap

.PHONY: all convert-svd convert-rdl convert-ipxact convert-json convert-csv export-converted clean

all: convert-svd convert-rdl convert-ipxact convert-json convert-csv export-converted

convert-svd:
	@echo "=== [format_conversion] Converting ARM CMSIS-SVD (.svd) to .rmt and .json ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f ../../svd/stm32_uart.svd --convert work/stm32_uart.rmt
	QT_QPA_PLATFORM=offscreen $(RMAP) -f ../../svd/stm32_uart.svd --convert work/stm32_uart.json
	@test -s work/stm32_uart.rmt && echo "✓ SVD to RMT conversion verified"
	@test -s work/stm32_uart.json && echo "✓ SVD to JSON conversion verified"

convert-rdl:
	@echo "=== [format_conversion] Converting SystemRDL (.rdl) to .rmt and .svd ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f ../../systemrdl/atxmega_spi.rdl --convert work/atxmega_spi.rmt
	QT_QPA_PLATFORM=offscreen $(RMAP) -f ../../systemrdl/atxmega_spi.rdl --convert work/atxmega_spi.svd
	@test -s work/atxmega_spi.rmt && echo "✓ SystemRDL to RMT conversion verified"
	@test -s work/atxmega_spi.svd && echo "✓ SystemRDL to SVD conversion verified"

convert-ipxact:
	@echo "=== [format_conversion] Converting IP-XACT IEEE 1685 (.xml) to .rmt and .svd ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f ../../ipxact/spi_ipxact.xml --convert work/spi_ipxact.rmt
	QT_QPA_PLATFORM=offscreen $(RMAP) -f ../../ipxact/spi_ipxact.xml --convert work/spi_ipxact.svd
	@test -s work/spi_ipxact.rmt && echo "✓ IP-XACT to RMT conversion verified"
	@test -s work/spi_ipxact.svd && echo "✓ IP-XACT to SVD conversion verified"
	QT_QPA_PLATFORM=offscreen $(RMAP) -f work/spi_ipxact.rmt -l
	@echo "✓ Converted IP-XACT model validation PASSED"

convert-json:
	@echo "=== [format_conversion] Converting JSON schema (.json) to .rmt ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f ../../json/sensor_hub.json --convert work/sensor_hub.rmt
	@test -s work/sensor_hub.rmt && echo "✓ JSON to RMT conversion verified"
	QT_QPA_PLATFORM=offscreen $(RMAP) -f work/sensor_hub.rmt -l
	@echo "✓ Converted JSON model validation PASSED"

convert-csv:
	@echo "=== [format_conversion] Converting RFC 4180 CSV (.csv) to .rmt and .json ==="
	@mkdir -p work
	QT_QPA_PLATFORM=offscreen $(RMAP) -f ../../csv/dma_controller.csv --convert work/dma_controller.rmt
	QT_QPA_PLATFORM=offscreen $(RMAP) -f ../../csv/dma_controller.csv --convert work/dma_controller.json
	@test -s work/dma_controller.rmt && echo "✓ CSV to RMT conversion verified"
	@test -s work/dma_controller.json && echo "✓ CSV to JSON conversion verified"

export-converted:
	@echo "=== [format_conversion] Exporting Register Models from Converted Maps ==="
	QT_QPA_PLATFORM=offscreen $(RMAP) -f work/sensor_hub.rmt -e -o export
	@test -s work/export/c/reg_map.h && echo "✓ C header generated from converted map"
	@test -s work/export/python/reg_map.py && echo "✓ Python driver generated from converted map"
	@test -s work/export/uvm/reg_model.sv && echo "✓ UVM model generated from converted map"
	@test -s work/export/rtl/reg_map.sv && echo "✓ RTL generated from converted map"
	@echo "✓ All format cross-conversion and export tests PASSED!"

clean:
	rm -rf work
