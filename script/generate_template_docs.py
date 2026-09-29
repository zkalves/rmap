#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
generate_template_docs.py - Generates individual documentation for rmap code generation templates.

Supports:
- Generating standalone documentation for each individual Inja template.
- Embedding WaveDrom timing diagrams (JSON source and rendered SVG/PNG images) for RTL and protocol templates.
- Cataloging context variables, interface signals, hardware sidebands, and software strobes.
"""

import argparse
import glob
import os
import sys
from pathlib import Path

# Insert script dir to import wavedrom_renderer
SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

from wavedrom_renderer import STANDARD_WAVEDROMS, export_wavedrom, render_all_standard_wavedroms

PROJECT_ROOT = SCRIPT_DIR.parent
TEMPLATES_DIR = PROJECT_ROOT / "templates"
DOCS_DIR = PROJECT_ROOT / "docs"
WAVEDROM_IMG_DIR = DOCS_DIR / "images" / "wavedrom"


# Comprehensive catalog of all 26 Inja templates
TEMPLATE_METADATA = {
    # 1. Synthesizable RTL
    "rtl/reg_map.sv.inja": {
        "title": "Synthesizable SystemVerilog Register File",
        "category": "RTL Synthesizable Hardware",
        "deliverable": "{out_dir}/{name}.sv",
        "standard": "IEEE 1800-2017 SystemVerilog",
        "description": "Bus-agnostic generic slave register file conforming to ASIC/FPGA HDL coding guidelines with 2-space indentation and clean synchronous resets.",
        "params": [
            ("DATA_WIDTH", "int", "32", "Register bus data width (8, 16, 32, 64, 128, 256, 512, 1024)"),
            ("ADDR_WIDTH", "int", "32", "Address bus width matching block address range"),
            ("PARAM_HW_PRECEDENCE", "bit", "1'b1", "Arbitration priority: 1 = HW update over SW write; 0 = SW over HW. Configurable via Inja custom parameter 'param_hw_precedence'")
        ],
        "ports": [
            ("clk_i", "input", "logic", "System clock"),
            ("rst_ni", "input", "logic", "Active-low asynchronous/synchronous reset"),
            ("bus_addr_i", "input", "logic [ADDR_WIDTH-1:0]", "Register word address"),
            ("bus_wdata_i", "input", "logic [DATA_WIDTH-1:0]", "Write data bus"),
            ("bus_wstrb_i", "input", "logic [DATA_WIDTH/8-1:0]", "Byte-level write enables"),
            ("bus_wr_en_i", "input", "logic", "Write strobe enable"),
            ("bus_rd_en_i", "input", "logic", "Read strobe enable"),
            ("bus_rdata_o", "output", "logic [DATA_WIDTH-1:0]", "Read data bus output"),
            ("sw_<reg>_wr_strobe_o", "output", "logic", "1-cycle pulse strobe asserted when software writes to register"),
            ("sw_<reg>_rd_strobe_o", "output", "logic", "1-cycle pulse strobe asserted when software reads from register"),
            ("sw_<reg>_<fld>_wr_strobe_o", "output", "logic", "1-cycle pulse strobe asserted when software writes to field"),
            ("sw_<reg>_<fld>_rd_strobe_o", "output", "logic", "1-cycle pulse strobe asserted when software reads from field"),
            ("hw_<reg>_<fld>_i", "input", "logic [WIDTH-1:0]", "Hardware input data for RO or WIRE fields"),
            ("hw_<reg>_<fld>_wd_i", "input", "logic [WIDTH-1:0]", "Hardware write data for RW/WO fields"),
            ("hw_<reg>_<fld>_we_i", "input", "logic", "Hardware write-enable strobe for RW/WO fields"),
            ("hw_<reg>_<fld>_tog_i", "input", "logic [WIDTH-1:0]", "Hardware toggle pulse mask for W1T fields"),
            ("hw_<reg>_<fld>_incr_i", "input", "logic", "Hardware pulse increment strobe for INCR fields"),
            ("hw_<reg>_<fld>_decr_i", "input", "logic", "Hardware pulse decrement strobe for DECR fields"),
            ("hw_<reg>_<fld>_o", "output", "logic [WIDTH-1:0]", "Live register field state observed by hardware logic (omitted for WIRE, WO, NA)")
        ],
        "wavedroms": [
            ("rtl_sw_write_strobe", "Software Write Cycle & Pulse Strobe Timing"),
            ("rtl_sw_read_strobe", "Software Read Cycle & Pulse Strobe Timing"),
            ("rtl_hw_update", "Hardware Logic Update with Write-Enable Condition"),
            ("rtl_hw_arbitration", "Concurrent Software vs. Hardware Arbitration Precedence"),
            ("rtl_w1c_cycle", "Write-1-to-Clear (W1C) Status Flag Latching & Clear")
        ]
    },
    "rtl/reg_map.v.inja": {
        "title": "Synthesizable Verilog-2001 Register File",
        "category": "RTL Synthesizable Hardware",
        "deliverable": "{out_dir}/{name}.v",
        "standard": "IEEE 1364-2001 Verilog",
        "description": "Legacy synthesizable Verilog implementation for older FPGA toolchains and ASIC synthesis flows with byte strobes and pulse triggers.",
        "params": [
            ("DATA_WIDTH", "integer", "32", "Register data width in bits"),
            ("ADDR_WIDTH", "integer", "32", "Address width in bits"),
            ("PARAM_HW_PRECEDENCE", "integer", "1", "1 = Hardware update priority; 0 = Software priority. Configurable via Inja custom parameter 'param_hw_precedence'")
        ],
        "ports": [
            ("clk_i", "input", "wire", "System clock"),
            ("rst_ni", "input", "wire", "Active-low reset"),
            ("bus_addr_i", "input", "wire [ADDR_WIDTH-1:0]", "Register address bus"),
            ("bus_wdata_i", "input", "wire [DATA_WIDTH-1:0]", "Bus write data"),
            ("bus_wstrb_i", "input", "wire [DATA_WIDTH/8-1:0]", "Byte write strobes"),
            ("bus_wr_en_i", "input", "wire", "Bus write enable"),
            ("bus_rd_en_i", "input", "wire", "Bus read enable"),
            ("bus_rdata_o", "output", "wire [DATA_WIDTH-1:0]", "Bus read data"),
            ("sw_<reg>_wr_strobe_o", "output", "wire", "Single-cycle write pulse strobe"),
            ("sw_<reg>_rd_strobe_o", "output", "wire", "Single-cycle read pulse strobe"),
            ("hw_<reg>_<fld>_i", "input", "wire [WIDTH-1:0]", "Hardware input data for RO or WIRE fields"),
            ("hw_<reg>_<fld>_wd_i", "input", "wire [WIDTH-1:0]", "Hardware write data for RW/WO fields"),
            ("hw_<reg>_<fld>_we_i", "input", "wire", "Hardware write-enable strobe for RW/WO fields"),
            ("hw_<reg>_<fld>_tog_i", "input", "wire [WIDTH-1:0]", "Hardware toggle pulse mask for W1T fields"),
            ("hw_<reg>_<fld>_incr_i", "input", "wire", "Hardware pulse increment strobe for INCR fields"),
            ("hw_<reg>_<fld>_decr_i", "input", "wire", "Hardware pulse decrement strobe for DECR fields"),
            ("hw_<reg>_<fld>_o", "output", "wire [WIDTH-1:0]", "Hardware data output (omitted for WIRE, WO, NA)")
        ],
        "wavedroms": [
            ("rtl_sw_write_strobe", "Verilog reg_map: Software Write Cycle with Byte Strobes"),
            ("rtl_hw_arbitration", "Verilog reg_map: Concurrent SW vs. HW Arbitration")
        ]
    },
    "rtl/reg_map.vhd.inja": {
        "title": "Synthesizable VHDL Register File",
        "category": "RTL Synthesizable Hardware",
        "deliverable": "{out_dir}/{name}.vhd",
        "standard": "IEEE 1076-1993/2008 VHDL",
        "description": "Synthesizable VHDL entity and architecture with standard std_logic_vector ports, generic parameterization, and synchronous process blocks.",
        "params": [
            ("DATA_WIDTH", "natural", "32", "Data bus width in bits"),
            ("ADDR_WIDTH", "natural", "32", "Address bus width in bits"),
            ("PARAM_HW_PRECEDENCE", "integer", "1", "Arbitration priority (1 = HW over SW; 0 = SW over HW). Configurable via Inja custom parameter 'param_hw_precedence'")
        ],
        "ports": [
            ("clk_i", "in", "std_logic", "Clock signal"),
            ("rst_ni", "in", "std_logic", "Active-low asynchronous reset"),
            ("bus_addr_i", "in", "std_logic_vector", "Address bus"),
            ("bus_wdata_i", "in", "std_logic_vector", "Write data bus"),
            ("bus_wstrb_i", "in", "std_logic_vector", "Byte write enables"),
            ("bus_wr_en_i", "in", "std_logic", "Write strobe enable"),
            ("bus_rd_en_i", "in", "std_logic", "Read strobe enable"),
            ("bus_rdata_o", "out", "std_logic_vector", "Read data bus output"),
            ("sw_<reg>_wr_strobe_o", "out", "std_logic", "Write pulse strobe output"),
            ("sw_<reg>_rd_strobe_o", "out", "std_logic", "Read pulse strobe output"),
            ("hw_<reg>_<fld>_i", "in", "std_logic_vector(WIDTH-1 downto 0)", "Hardware input data for RO or WIRE fields"),
            ("hw_<reg>_<fld>_wd_i", "in", "std_logic_vector(WIDTH-1 downto 0)", "Hardware write data for RW/WO fields"),
            ("hw_<reg>_<fld>_we_i", "in", "std_logic", "Hardware write-enable strobe for RW/WO fields"),
            ("hw_<reg>_<fld>_tog_i", "in", "std_logic_vector(WIDTH-1 downto 0)", "Hardware toggle pulse mask for W1T fields"),
            ("hw_<reg>_<fld>_incr_i", "in", "std_logic", "Hardware pulse increment strobe for INCR fields"),
            ("hw_<reg>_<fld>_decr_i", "in", "std_logic", "Hardware pulse decrement strobe for DECR fields"),
            ("hw_<reg>_<fld>_o", "out", "std_logic_vector(WIDTH-1 downto 0)", "Live field output to hardware logic (omitted for WIRE, WO, NA)")
        ],
        "wavedroms": [
            ("rtl_sw_write_strobe", "VHDL reg_map: Software Write & Strobe Timing")
        ]
    },
    "rtl/apb_reg_file.sv.inja": {
        "title": "Synthesizable APB4 Register Slave Wrapper",
        "category": "RTL Synthesizable Hardware",
        "deliverable": "{out_dir}/{name}_apb.sv",
        "standard": "AMBA 4 APB (APB4 v2.0)",
        "description": "AMBA 4 APB synthesizable bridge wrapping the generic register file with setup and access phase state logic, byte strobing, and error responses.",
        "params": [
            ("ADDR_WIDTH", "int", "32", "APB address width in bits"),
            ("DATA_WIDTH", "int", "32", "APB data width in bits")
        ],
        "ports": [
            ("pclk", "input", "logic", "APB bus clock"),
            ("presetn", "input", "logic", "Active-low APB reset"),
            ("paddr", "input", "logic [ADDR_WIDTH-1:0]", "APB byte address"),
            ("psel", "input", "logic", "APB slave select"),
            ("penable", "input", "logic", "APB enable strobe"),
            ("pwrite", "input", "logic", "1 = Write transaction; 0 = Read"),
            ("pwdata", "input", "logic [DATA_WIDTH-1:0]", "APB write data"),
            ("pstrb", "input", "logic [DATA_WIDTH/8-1:0]", "APB write byte strobes"),
            ("pready", "output", "logic", "APB ready acknowledgement"),
            ("prdata", "output", "logic [DATA_WIDTH-1:0]", "APB read data"),
            ("pslverr", "output", "logic", "APB slave error (unmapped address / decode error)")
        ],
        "wavedroms": [
            ("apb_write_read_protocol", "AMBA 4 APB: Two-Phase Write and Read Bus Transactions")
        ]
    },
    "rtl/axil_reg_file.sv.inja": {
        "title": "Synthesizable AXI4-Lite Register Slave Wrapper",
        "category": "RTL Synthesizable Hardware",
        "deliverable": "{out_dir}/{name}_axil.sv",
        "standard": "AMBA 4 AXI4-Lite",
        "description": "AMBA 4 AXI4-Lite synthesizable slave wrapper implementing 5 independent handshake channels (AW, W, B, AR, R) around the generic register file.",
        "params": [
            ("ADDR_WIDTH", "int", "32", "AXI address bus width in bits"),
            ("DATA_WIDTH", "int", "32", "AXI data bus width in bits")
        ],
        "ports": [
            ("s_axi_aclk", "input", "logic", "AXI clock"),
            ("s_axi_aresetn", "input", "logic", "Active-low AXI reset"),
            ("s_axi_awaddr", "input", "logic [ADDR_WIDTH-1:0]", "Write address"),
            ("s_axi_awvalid", "input", "logic", "Write address valid"),
            ("s_axi_awready", "output", "logic", "Write address ready"),
            ("s_axi_wdata", "input", "logic [DATA_WIDTH-1:0]", "Write data bus"),
            ("s_axi_wstrb", "input", "logic [DATA_WIDTH/8-1:0]", "Write byte strobes"),
            ("s_axi_wvalid", "input", "logic", "Write data valid"),
            ("s_axi_wready", "output", "logic", "Write data ready"),
            ("s_axi_bresp", "output", "logic [1:0]", "Write response (2'b00 = OKAY)"),
            ("s_axi_bvalid", "output", "logic", "Write response valid"),
            ("s_axi_bready", "input", "logic", "Write response ready"),
            ("s_axi_araddr", "input", "logic [ADDR_WIDTH-1:0]", "Read address"),
            ("s_axi_arvalid", "input", "logic", "Read address valid"),
            ("s_axi_arready", "output", "logic", "Read address ready"),
            ("s_axi_rdata", "output", "logic [DATA_WIDTH-1:0]", "Read data output"),
            ("s_axi_rresp", "output", "logic [1:0]", "Read response (2'b00 = OKAY)"),
            ("s_axi_rvalid", "output", "logic", "Read data valid"),
            ("s_axi_rready", "input", "logic", "Read data ready")
        ],
        "wavedroms": [
            ("axil_write_read_protocol", "AMBA AXI4-Lite: Five-Channel Handshake Transactions")
        ]
    },
    "rtl/reg_map_sva.sv.inja": {
        "title": "Formal & Dynamic SystemVerilog Assertions (SVA)",
        "category": "RTL Synthesizable Hardware",
        "deliverable": "{out_dir}/{name}_sva.sv",
        "standard": "IEEE 1800-2017 SVA",
        "description": "Formal and simulation SVA assertion module bindable directly to the RTL register file to formally verify strobe invariants, reset integrity, and arbitration rules.",
        "params": [],
        "ports": [],
        "wavedroms": [
            ("sva_strobe_pulse", "SVA Verification: Single-Cycle Software Strobe Pulse Assertion")
        ]
    },
    "rtl_tb/tb_reg_map.sv.inja": {
        "title": "Self-Checking RTL Testbench",
        "category": "Verification Testbenches",
        "deliverable": "{out_dir}/tb_{name}.sv",
        "standard": "IEEE 1800-2017 SystemVerilog",
        "description": "Standalone self-checking testbench exercising reset values, bitfield masks, hardware writes, and read-only/write-1-to-clear behaviors with Icarus Verilog or Verilator.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },

    # 2. UVM & pyuvm
    "uvm/reg_model.sv.inja": {
        "title": "UVM SystemVerilog Register Model",
        "category": "UVM Verification Suites",
        "deliverable": "{out_dir}/{name}_pkg.sv",
        "standard": "IEEE 1800.2 / Accellera UVM",
        "description": "Complete UVM register package providing uvm_reg_block, uvm_reg, uvm_reg_field, multi-map instances (apb_map, axi_map), memory windows (uvm_mem), and coverage models.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "uvm_tb/reg_bus_if.sv.inja": {
        "title": "UVM Generic Bus Interface",
        "category": "UVM Verification Suites",
        "deliverable": "{out_dir}/reg_bus_if.sv",
        "standard": "IEEE 1800-2017 SystemVerilog",
        "description": "Generic bidirectional bus interface pin bundle connecting UVM bus driver and monitor VIP to the DUT.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "uvm_tb/reg_bus_pkg.sv.inja": {
        "title": "UVM Bus VIP Agent Package",
        "category": "UVM Verification Suites",
        "deliverable": "{out_dir}/reg_bus_pkg.sv",
        "standard": "IEEE 1800.2 UVM",
        "description": "UVM VIP package providing bus driver, monitor, sequencer, agent, and uvm_reg_adapter implementations.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "uvm_tb/reg_env.sv.inja": {
        "title": "UVM Register Verification Environment",
        "category": "UVM Verification Suites",
        "deliverable": "{out_dir}/reg_env.sv",
        "standard": "IEEE 1800.2 UVM",
        "description": "Top-level UVM verification environment connecting the register model, bus agent, predictor, and coverage collectors.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "uvm_tb/reg_tests.sv.inja": {
        "title": "UVM Built-in Test Sequences",
        "category": "UVM Verification Suites",
        "deliverable": "{out_dir}/reg_tests.sv",
        "standard": "IEEE 1800.2 UVM",
        "description": "UVM test sequence suite executing reset check (uvm_reg_hw_reset_seq), bit-bash (uvm_reg_bit_bash_seq), and read/write access (uvm_reg_access_seq).",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "uvm_tb/tb_top.sv.inja": {
        "title": "UVM Top-Level Testbench Harness",
        "category": "UVM Verification Suites",
        "deliverable": "{out_dir}/tb_top.sv",
        "standard": "IEEE 1800-2017 SystemVerilog",
        "description": "Top-level simulation harness instantiating the DUT, generating clock and reset, setting the virtual interface into uvm_config_db, and running run_test().",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "pyuvm_tb/tb_pyuvm.py.inja": {
        "title": "Open-Source Python UVM Testbench",
        "category": "UVM Verification Suites",
        "deliverable": "{out_dir}/tb_pyuvm.py",
        "standard": "pyuvm / Cocotb",
        "description": "Python verification environment executing register tests headlessly with open-source simulators (Icarus Verilog, Verilator) without commercial EDA licenses.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },

    # 3. Software, Firmware & Drivers
    "c/reg_map.h.inja": {
        "title": "C/C++ Firmware Header",
        "category": "Firmware & Software Drivers",
        "deliverable": "{out_dir}/{name}.h",
        "standard": "ANSI C99 / C++11",
        "description": "Embedded C header providing register address offset macros, bitfield masks, bitfield shifts, packed volatile structs, and bitfield extraction/update macros.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "rust/reg_map.rs.inja": {
        "title": "Rust Peripheral Access Crate (PAC)",
        "category": "Firmware & Software Drivers",
        "deliverable": "{out_dir}/{name}.rs",
        "standard": "Rust 2021 Edition",
        "description": "Memory-safe Rust Peripheral Access Crate with #[repr(C)] register block structures and type-safe read(), write(), and modify() accessors.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "python/reg_map.py.inja": {
        "title": "Python Bring-Up Register Driver",
        "category": "Firmware & Software Drivers",
        "deliverable": "{out_dir}/{name}.py",
        "standard": "Python 3.8+",
        "description": "Standalone Python register driver class with named bitfield getters/setters and pluggable bus adapters for PyUVM, Cocotb, PyFTDI, or PySerial.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },

    # 4. Documentation & Specifications
    "html/reg_doc.html.inja": {
        "title": "Interactive HTML Specification",
        "category": "Documentation & Specifications",
        "deliverable": "{out_dir}/{name}.html",
        "standard": "HTML5 / Responsive CSS",
        "description": "Single-page responsive HTML report with live client-side search bar and graphical color-coded bitfield slice visualizers.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "markdown/reg_doc.md.inja": {
        "title": "Markdown Documentation Specification",
        "category": "Documentation & Specifications",
        "deliverable": "{out_dir}/{name}.md",
        "standard": "GitHub-Flavored Markdown (GFM)",
        "description": "Clean Markdown register specification tables with block navigation anchors, bitfield tables, and dynamically filtered feature summaries.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "markdown/reg_features.md.inja": {
        "title": "Markdown Architecture & Feature Guide",
        "category": "Documentation & Specifications",
        "deliverable": "{out_dir}/{name}_features.md",
        "standard": "GitHub-Flavored Markdown (GFM)",
        "description": "Dedicated guide detailing IEEE 1800.2 access policies, hardware sidebands, and volatile semantics dynamically filtered to only active features implemented in the register map.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "asciidoctor/reg_doc.adoc.inja": {
        "title": "AsciiDoctor Documentation Specification",
        "category": "Documentation & Specifications",
        "deliverable": "{out_dir}/{name}.adoc",
        "standard": "AsciiDoctor / AsciiDoc",
        "description": "Modern AsciiDoctor specification with table of contents, block anchors, register sections, and conditional ifndef directives for feature toggling.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },

    # 5. Interchange Formats & Simulation
    "systemrdl/reg_map.rdl.inja": {
        "title": "Accellera SystemRDL 2.0 Register Model",
        "category": "Format Interchange & Verification",
        "deliverable": "{out_dir}/{name}.rdl",
        "standard": "Accellera SystemRDL 2.0",
        "description": "Standard SystemRDL 2.0 addrmap and regfile component specification defining hierarchical register trees, SW/HW access properties, and reset states.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "ipxact/reg_map.xml.inja": {
        "title": "IP-XACT IEEE 1685 Component Schema",
        "category": "Format Interchange & Verification",
        "deliverable": "{out_dir}/{name}_ipxact.xml",
        "standard": "IEEE 1685-2014 / 2022 IP-XACT",
        "description": "Industry-standard XML component packaging defining ipxact:memoryMaps, address blocks, register structures, and bitfield ranges for EDA tooling.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "svd/reg_map.xml.inja": {
        "title": "ARM CMSIS-SVD Peripheral XML",
        "category": "Format Interchange & Verification",
        "deliverable": "{out_dir}/{name}.svd",
        "standard": "ARM CMSIS-SVD v1.3+",
        "description": "Cortex-M microcontroller peripheral specification formatted for hardware debuggers (Keil, IAR, VS Code Cortex-Debug) and vendor SDK code generators.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "json/reg_map.json.inja": {
        "title": "Structured JSON Schema Export",
        "category": "Format Interchange & Verification",
        "deliverable": "{out_dir}/{name}.json",
        "standard": "JSON Schema",
        "description": "Structured JSON schema representation of the register hierarchy for custom Python automation, CI pipelines, and external EDA tooling.",
        "params": [],
        "ports": [],
        "wavedroms": []
    },
    "sim/Makefile.inja": {
        "title": "Multi-Tool Simulation Makefile",
        "category": "Format Interchange & Verification",
        "deliverable": "{out_dir}/Makefile",
        "standard": "GNU Make",
        "description": "Automated test runner targeting Icarus Verilog (sim-rtl), Verilator (sim-verilator), Cocotb/pyuvm (sim-pyuvm), and commercial EDA suites (sim-uvm).",
        "params": [],
        "ports": [],
        "wavedroms": []
    }
}


def generate_template_markdown(tmpl_rel_path: str, img_rel_prefix: str = "../images/wavedrom") -> str:
    meta = TEMPLATE_METADATA.get(tmpl_rel_path)
    if not meta:
        return f"# Template Documentation: `{tmpl_rel_path}`\n\nNo detailed metadata registered."

    md = []
    md.append(f"### `{tmpl_rel_path}`: {meta['title']}")
    md.append(f"- **Standard / Target**: {meta['standard']}")
    md.append(f"- **Category**: {meta['category']}")
    md.append(f"- **Default Deliverable Path**: `{meta['deliverable']}`")
    md.append(f"- **Description**: {meta['description']}")

    # Parameters table
    if meta.get("params"):
        md.append("\n#### Generic Parameters & Configurations\n")
        md.append("| Parameter | Type | Default | Description |")
        md.append("| :--- | :--- | :--- | :--- |")
        for p_name, p_type, p_def, p_desc in meta["params"]:
            md.append(f"| `{p_name}` | `{p_type}` | `{p_def}` | {p_desc} |")

    # Ports table
    if meta.get("ports"):
        md.append("\n#### Interface Signals & Ports\n")
        md.append("| Signal Name | Direction | Type / Width | Description |")
        md.append("| :--- | :--- | :--- | :--- |")
        for s_name, s_dir, s_type, s_desc in meta["ports"]:
            md.append(f"| `{s_name}` | {s_dir.upper()} | `{s_type}` | {s_desc} |")

    # WaveDrom timing diagrams
    if meta.get("wavedroms"):
        md.append("\n#### Protocol Timing Diagrams (WaveDrom)\n")
        for wd_key, wd_title in meta["wavedroms"]:
            spec = STANDARD_WAVEDROMS.get(wd_key)
            if not spec:
                continue
            md.append(f"##### {wd_title}\n")
            img_path = f"{img_rel_prefix}/{wd_key}.png"
            md.append(f"![{wd_title}]({img_path})\n")
            md.append("<details><summary>WaveDrom JSON Source (Timing Specification)</summary>\n")
            md.append("```wavedrom")
            import json
            md.append(json.dumps(spec, indent=2))
            md.append("```\n</details>\n")

    return "\n".join(md)


def main():
    parser = argparse.ArgumentParser(description="Generate individual rmap template documentation")
    parser.add_argument("--template", help="Relative template path (e.g. rtl/reg_map.sv.inja)")
    parser.add_argument("--all", action="store_true", help="Generate documentation for all 26 templates")
    parser.add_argument("--category", help="Generate documentation for templates in category (rtl, uvm, etc.)")
    parser.add_argument("--wavedrom", action="store_true", help="Generate all WaveDrom SVG and PNG timing diagrams")
    parser.add_argument("--out-dir", default="", help="Output directory to write markdown files")

    args = parser.parse_args()

    if args.wavedrom:
        print(f"--> Rendering WaveDrom timing diagrams into: {WAVEDROM_IMG_DIR}")
        render_all_standard_wavedroms(str(WAVEDROM_IMG_DIR))
        print("✓ All WaveDrom diagrams generated.")
        if not args.template and not args.all and not args.category:
            sys.exit(0)

    if args.template:
        t_key = args.template
        if not t_key.endswith(".inja"):
            t_key += ".inja"
        # Strip leading templates/ if supplied
        if t_key.startswith("templates/"):
            t_key = t_key[10:]

        doc = generate_template_markdown(t_key)
        if args.out_dir:
            out_file = Path(args.out_dir) / f"{Path(t_key).stem}.md"
            out_file.parent.mkdir(parents=True, exist_ok=True)
            with open(out_file, "w", encoding="utf-8") as f:
                f.write(doc)
            print(f"✓ Generated: {out_file}")
        else:
            print(doc)
        sys.exit(0)

    if args.category:
        cat_filter = args.category.strip().lower()
        matched = {k: v for k, v in TEMPLATE_METADATA.items() if k.startswith(cat_filter + "/") or cat_filter in v["category"].lower()}
        if not matched:
            print(f"No templates found matching category '{args.category}'", file=sys.stderr)
            sys.exit(1)
        out_base = Path(args.out_dir) if args.out_dir else DOCS_DIR / "user" / "templates"
        out_base.mkdir(parents=True, exist_ok=True)
        print(f"--> Generating individual documentation for {len(matched)} templates in category '{args.category}' into: {out_base}")
        for t_key in sorted(matched.keys()):
            cat, filename = t_key.split("/", 1)
            doc = generate_template_markdown(t_key, img_rel_prefix="../../../images/wavedrom")
            sub_path = out_base / cat / f"{filename[:-5]}.md"
            sub_path.parent.mkdir(parents=True, exist_ok=True)
            with open(sub_path, "w", encoding="utf-8") as f:
                f.write(doc)
            print(f"  ✓ {t_key} -> {cat}/{sub_path.name}")
        sys.exit(0)

    if args.all:
        out_base = Path(args.out_dir) if args.out_dir else DOCS_DIR / "user" / "templates"
        out_base.mkdir(parents=True, exist_ok=True)
        print(f"--> Generating individual documentation for all {len(TEMPLATE_METADATA)} templates into: {out_base}")
        index_entries = []
        for t_key in sorted(TEMPLATE_METADATA.keys()):
            cat, filename = t_key.split("/", 1)
            doc = generate_template_markdown(t_key, img_rel_prefix="../../../images/wavedrom")
            sub_path = out_base / cat / f"{filename[:-5]}.md"
            sub_path.parent.mkdir(parents=True, exist_ok=True)
            with open(sub_path, "w", encoding="utf-8") as f:
                f.write(doc)
            print(f"  ✓ {t_key} -> {cat}/{sub_path.name}")
            meta = TEMPLATE_METADATA[t_key]
            index_entries.append((cat, t_key, meta["title"], f"{cat}/{sub_path.name}"))

        index_file = out_base / "index.md"
        with open(index_file, "w", encoding="utf-8") as f:
            f.write("# Individual Template Deliverables Catalog {#template_catalog}\n\n")
            f.write("This directory contains standalone, isolated specifications for each of the 26 code generation templates in **rmap**.\n\n")
            f.write("| Category | Template Source | Title | Standalone Spec |\n")
            f.write("| :--- | :--- | :--- | :--- |\n")
            for cat, t_key, title, link in index_entries:
                f.write(f"| `{cat}` | `{t_key}` | {title} | [{title}]({link}) |\n")
        print(f"✓ Generated catalog index: {index_file}")
        print("✓ Completed generating all template documentation.")
        sys.exit(0)

    parser.print_help()


if __name__ == "__main__":
    main()
