#!/usr/bin/env python3
"""
Comprehensive Automated Template Verification Suite for rmap.
Validates code generation and generated artifact correctness across all 11 templates:
c, rtl, uvm, rust, python, html, markdown, systemrdl, ipxact, svd, json.

Usage:
    python3 tests/test_template.py <template_name>
    python3 tests/test_template.py all
"""

import sys
import os
import shutil
import subprocess
import json
import re
import importlib.util
from html.parser import HTMLParser
import xml.etree.ElementTree as ET
import py_compile

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DEFAULT_BIN = os.path.join(PROJECT_ROOT, "build", "bin", "rmap")
WORK_BASE = os.path.join(PROJECT_ROOT, "work", "test_templates")

class StrictHTMLParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.tags = []
        self.tag_counts = {}

    def handle_starttag(self, tag, attrs):
        self.tags.append(tag)
        self.tag_counts[tag] = self.tag_counts.get(tag, 0) + 1


def find_rmap_binary():
    bin_path = os.environ.get("RMAP_BIN", DEFAULT_BIN)
    if not os.path.exists(bin_path):
        alt_bin = shutil.which("rmap")
        if alt_bin:
            return alt_bin
        print(f"Error: rmap binary not found at '{bin_path}'. Build it first with 'make rmap'.", file=sys.stderr)
        sys.exit(1)
    return bin_path


def run_command(cmd, check=True, cwd=None, env=None):
    merged_env = os.environ.copy()
    merged_env["QT_QPA_PLATFORM"] = "offscreen"
    if env:
        merged_env.update(env)
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, cwd=cwd, env=merged_env)
    if check and proc.returncode != 0:
        print(f"Command failed: {' '.join(cmd)}", file=sys.stderr)
        print(f"Stdout:\n{proc.stdout}", file=sys.stderr)
        print(f"Stderr:\n{proc.stderr}", file=sys.stderr)
        raise RuntimeError(f"Command returned {proc.returncode}")
    return proc


def export_examples(template_name, rmap_bin, target_work_dir):
    """Export comprehensive and spi examples into target_work_dir."""
    os.makedirs(target_work_dir, exist_ok=True)
    comp_rmt = os.path.join(PROJECT_ROOT, "examples", "comprehensive.rmt")
    spi_rmt = os.path.join(PROJECT_ROOT, "examples", "spi.rmt")
    gap_rmt = os.path.join(PROJECT_ROOT, "examples", "address_gap_example.rmt")

    comp_out = os.path.join(target_work_dir, "comprehensive")
    spi_out = os.path.join(target_work_dir, "spi")
    gap_out = os.path.join(target_work_dir, "gap")

    os.makedirs(comp_out, exist_ok=True)
    os.makedirs(spi_out, exist_ok=True)
    os.makedirs(gap_out, exist_ok=True)

    run_command([rmap_bin, "-f", comp_rmt, "-e", "-o", comp_out], cwd=PROJECT_ROOT)
    run_command([rmap_bin, "-f", spi_rmt, "-e", "-o", spi_out], cwd=PROJECT_ROOT)
    run_command([rmap_bin, "-f", gap_rmt, "-e", "-o", gap_out], cwd=PROJECT_ROOT)

    return comp_out, spi_out, gap_out


# =============================================================================
# 1. C Template Validator
# =============================================================================
def test_c(rmap_bin, work_dir):
    print("Testing 'c' template (C/C++ Firmware Header)...")
    comp_out, spi_out, gap_out = export_examples("c", rmap_bin, work_dir)

    c_comp_file = os.path.join(comp_out, "c", "reg_map.h")
    c_spi_file = os.path.join(spi_out, "c", "reg_map.h")
    c_gap_file = os.path.join(gap_out, "c", "sparse_reg_map.h")

    for f in [c_comp_file, c_spi_file, c_gap_file]:
        assert os.path.isfile(f), f"Expected C header file '{f}' not generated!"

    with open(c_comp_file, "r") as f:
        comp_content = f.read()
    with open(c_gap_file, "r") as f:
        gap_content = f.read()

    # Structural assertions
    assert "#ifndef" in comp_content and "REG_MAP_H" in comp_content
    assert '#include <stdint.h>' in comp_content
    assert 'extern "C"' in comp_content
    assert '#define RMAP_REG_GET(val, mask, shift)' in comp_content
    assert '#define RMAP_REG_SET(val, mask, shift, fv)' in comp_content
    assert 'CORE_SUBSYSTEM_CONTROL_OFFSET' in comp_content
    assert 'CORE_SUBSYSTEM_CONTROL_ENABLE_MASK' in comp_content
    assert 'CORE_SUBSYSTEM_CONTROL_ENABLE_SHIFT' in comp_content
    assert 'CORE_SUBSYSTEM_CONTROL_ENABLE_GET' in comp_content
    assert 'CORE_SUBSYSTEM_CONTROL_ENABLE_SET' in comp_content
    assert 'typedef struct {' in comp_content
    assert 'core_subsystem_regs_t;' in comp_content
    assert 'volatile uint32_t control;' in comp_content

    # Gap padding assertions
    assert '_reserved_' in gap_content, "Expected reserved gap padding words in sparse register map!"
    assert 'sparse_device_regs_t;' in gap_content

    # C99 and C++17 compilation and execution harness
    test_harness_c = os.path.join(work_dir, "test_c_harness.c")
    with open(test_harness_c, "w") as f:
        f.write(r"""
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "comprehensive/c/reg_map.h"
#include "gap/c/sparse_reg_map.h"

int main(void) {
    // 1. Verify register offset macros
    assert(CORE_SUBSYSTEM_CONTROL_OFFSET == 0x0000);
    assert(CORE_SUBSYSTEM_STATUS_FLAGS_OFFSET == 0x0004);
    assert(CORE_SUBSYSTEM_SPECIAL_ACCESS_OFFSET == 0x0008);

    // 2. Verify bitfield mask and shift macros
    assert(CORE_SUBSYSTEM_CONTROL_ENABLE_MASK == 0x1);
    assert(CORE_SUBSYSTEM_CONTROL_ENABLE_SHIFT == 0);
    assert(CORE_SUBSYSTEM_CONTROL_MODE_MASK == 0xE);
    assert(CORE_SUBSYSTEM_CONTROL_MODE_SHIFT == 1);
    assert(CORE_SUBSYSTEM_CONTROL_TIMEOUT_MASK == 0xFF0);
    assert(CORE_SUBSYSTEM_CONTROL_TIMEOUT_SHIFT == 4);

    // 3. Verify RMAP_REG_GET and RMAP_REG_SET functionality
    uint32_t reg = 0;
    reg = CORE_SUBSYSTEM_CONTROL_ENABLE_SET(reg, 1);
    assert(CORE_SUBSYSTEM_CONTROL_ENABLE_GET(reg) == 1);
    assert(reg == 0x1);

    reg = CORE_SUBSYSTEM_CONTROL_MODE_SET(reg, 5); // 5 << 1 = 10 (0xA)
    assert(CORE_SUBSYSTEM_CONTROL_MODE_GET(reg) == 5);
    assert(CORE_SUBSYSTEM_CONTROL_ENABLE_GET(reg) == 1); // preserved

    reg = CORE_SUBSYSTEM_CONTROL_TIMEOUT_SET(reg, 0x42); // 0x42 << 4 = 0x420
    assert(CORE_SUBSYSTEM_CONTROL_TIMEOUT_GET(reg) == 0x42);
    assert(CORE_SUBSYSTEM_CONTROL_MODE_GET(reg) == 5);
    assert(CORE_SUBSYSTEM_CONTROL_ENABLE_GET(reg) == 1);

    // 4. Verify sparse memory map struct offsets match exact register addresses
    assert(offsetof(sparse_device_regs_t, ctrl) == 0x0000);
    assert(offsetof(sparse_device_regs_t, status) == 0x0004);
    assert(offsetof(sparse_device_regs_t, tx_fifo) == 0x0020);
    assert(offsetof(sparse_device_regs_t, rx_fifo) == 0x0024);
    assert(offsetof(sparse_device_regs_t, irq_enable) == 0x0040);
    assert(offsetof(sparse_device_regs_t, irq_status) == 0x0044);
    assert(offsetof(sparse_device_regs_t, periph_id) == 0x0080);

    printf("C Template Verification Harness Passed!\n");
    return 0;
}
""")

    gcc_bin = shutil.which("gcc")
    if gcc_bin:
        c_bin = os.path.join(work_dir, "test_c_harness")
        run_command([gcc_bin, "-Wall", "-Wextra", "-Werror", "-pedantic", "-std=c99",
                     f"-I{work_dir}", test_harness_c, "-o", c_bin])
        run_command([c_bin])
        print("  ✓ GCC C99 build and assertions passed.")

    gpp_bin = shutil.which("g++")
    if gpp_bin:
        cpp_bin = os.path.join(work_dir, "test_cpp_harness")
        run_command([gpp_bin, "-Wall", "-Wextra", "-Werror", "-pedantic", "-std=c++17",
                     f"-I{work_dir}", test_harness_c, "-o", cpp_bin])
        run_command([cpp_bin])
        print("  ✓ G++ C++17 build and assertions passed.")

    print("✓ C template verified successfully.\n")


# =============================================================================
# 2. RTL Template Validator (Synthesizable SystemVerilog)
# =============================================================================
def test_rtl(rmap_bin, work_dir):
    print("Testing 'rtl' template (Synthesizable SystemVerilog Register File)...")
    comp_out, spi_out, _ = export_examples("rtl", rmap_bin, work_dir)

    rtl_comp_file = os.path.join(comp_out, "rtl", "reg_map.sv")
    rtl_spi_file = os.path.join(spi_out, "rtl", "reg_map.sv")

    assert os.path.isfile(rtl_comp_file), f"RTL output '{rtl_comp_file}' not found!"
    assert os.path.isfile(rtl_spi_file), f"RTL output '{rtl_spi_file}' not found!"

    with open(rtl_comp_file, "r") as f:
        content = f.read()

    # Structural assertions
    assert "timeunit 1ns;" in content
    assert "timeprecision 1ps;" in content
    assert "module core_subsystem_reg_file" in content
    assert "input  logic                      clk_i" in content
    assert "input  logic                      rst_ni" in content
    assert "input  logic                      wr_en_i" in content
    assert "input  logic                      rd_en_i" in content
    assert "input  logic [ADDR_WIDTH-1:0]     addr_i" in content
    assert "input  logic [DATA_WIDTH-1:0]     wdata_i" in content
    assert "input  logic [STRB_WIDTH-1:0]     wstrb_i" in content
    assert "output logic [DATA_WIDTH-1:0]     rdata_o" in content
    assert "output logic                      ready_o" in content
    assert "output logic                      irq_o" in content

    # Sideband hardware ports
    assert "output logic                      sw_control_wr_strobe_o" in content
    assert "output logic                      sw_control_rd_strobe_o" in content
    assert "hw_control_enable_o" in content
    assert "hw_status_flags_busy_i" in content
    assert "hw_status_flags_irq_pending_set_i" in content
    assert "hw_status_flags_irq_pending_o" in content

    # Address localparams
    assert "localparam logic [ADDR_WIDTH-1:0] ADDR_CONTROL = 0x0000;" in content
    assert "localparam logic [ADDR_WIDTH-1:0] ADDR_STATUS_FLAGS = 0x0004;" in content

    # Byte-strobe qualified next-state logic
    assert "wstrb_i[b]" in content
    assert "always_comb begin : proc_control_next" in content
    assert "always_comb begin : proc_read_decode" in content
    assert "always_ff @(posedge clk_i or negedge rst_ni)" in content

    # Verilator lint if installed
    verilator_bin = shutil.which("verilator")
    if verilator_bin:
        run_command([verilator_bin, "--lint-only", "-Wno-fatal", "-Wno-DECLFILENAME", rtl_comp_file])
        run_command([verilator_bin, "--lint-only", "-Wno-fatal", "-Wno-DECLFILENAME", rtl_spi_file])
        print("  ✓ Verilator lint checks passed.")

    # Icarus Verilog if installed
    iverilog_bin = shutil.which("iverilog")
    if iverilog_bin:
        run_command([iverilog_bin, "-g2012", "-t", "null", rtl_comp_file])
        run_command([iverilog_bin, "-g2012", "-t", "null", rtl_spi_file])
        print("  ✓ Icarus Verilog syntax checks passed.")

    print("✓ RTL template verified successfully.\n")


# =============================================================================
# 3. UVM Template Validator (UVM Register Model)
# =============================================================================
def test_uvm(rmap_bin, work_dir):
    print("Testing 'uvm' template (UVM Register Model)...")
    comp_out, spi_out, _ = export_examples("uvm", rmap_bin, work_dir)

    uvm_comp_file = os.path.join(comp_out, "uvm", "reg_model.sv")
    uvm_spi_file = os.path.join(spi_out, "uvm", "reg_model.sv")

    assert os.path.isfile(uvm_comp_file), f"UVM output '{uvm_comp_file}' not found!"
    assert os.path.isfile(uvm_spi_file), f"UVM output '{uvm_spi_file}' not found!"

    with open(uvm_comp_file, "r") as f:
        content = f.read()

    # Structure assertions
    assert "class core_subsystem_reg_block extends uvm_reg_block;" in content
    assert "`uvm_object_utils(core_subsystem_reg_block)" in content
    assert "class control_reg extends uvm_reg;" in content
    assert "`uvm_object_utils(control_reg)" in content
    assert "rand uvm_reg_field enable;" in content
    assert "rand uvm_reg_field mode;" in content
    assert "super.new(name, 32, UVM_CVR_ALL);" in content

    # Field configure calls
    assert 'this.enable = uvm_reg_field::type_id::create("ENABLE");' in content
    assert 'this.enable.configure(this, 1, 0, "RW", false, 0x0001, true, true, 1);' in content

    # Map creation and registration
    assert 'this.default_map = create_map("default_map", 0, 4, UVM_LITTLE_ENDIAN);' in content
    assert 'this.control.add_hdl_path_slice("reg_control_q", 0, 32);' in content
    assert 'this.default_map.add_reg(this.control, 0x0000, "RW");' in content

    print("✓ UVM template verified successfully.\n")


# =============================================================================
# 4. Rust Template Validator (Rust PAC)
# =============================================================================
def test_rust(rmap_bin, work_dir):
    print("Testing 'rust' template (Rust Peripheral Access Crate)...")
    comp_out, spi_out, _ = export_examples("rust", rmap_bin, work_dir)

    rust_comp_file = os.path.join(comp_out, "rust", "reg_map.rs")
    rust_spi_file = os.path.join(spi_out, "rust", "reg_map.rs")

    assert os.path.isfile(rust_comp_file), f"Rust output '{rust_comp_file}' not found!"
    assert os.path.isfile(rust_spi_file), f"Rust output '{rust_spi_file}' not found!"

    with open(rust_comp_file, "r") as f:
        content = f.read()

    # Structural assertions
    assert "#![no_std]" in content
    assert "use core::ptr::{read_volatile, write_volatile};" in content
    assert "#[repr(C)]" in content
    assert "pub struct CORE_SUBSYSTEM {" in content
    assert "pub control: CONTROL_REG," in content
    assert "#[repr(transparent)]" in content
    assert "pub struct CONTROL_REG {" in content
    assert "register: u32," in content
    assert "pub const OFFSET: usize = 0;" in content
    assert "pub unsafe fn read(&self) -> u32" in content
    assert "pub unsafe fn write(&mut self, val: u32)" in content
    assert "pub unsafe fn modify<F>(&mut self, f: F)" in content
    assert "pub const ENABLE_MASK: u32 = 0x1;" in content
    assert "pub const ENABLE_OFFSET: u32 = 0;" in content
    assert "pub fn get_enable(val: u32) -> u32" in content
    assert "pub fn set_enable(val: u32, fld_val: u32) -> u32" in content

    rustc_bin = shutil.which("rustc")
    if rustc_bin:
        # 1. Compile generated code as rlib
        rlib_out = os.path.join(work_dir, "libreg_map.rlib")
        run_command([rustc_bin, "--crate-type", "lib", "--crate-name", "reg_map", "--edition", "2021", rust_comp_file, "-o", rlib_out])
        print("  ✓ rustc compilation to rlib passed.")

        # 2. Compile and run Rust functional test harness linking to rlib
        test_harness_rs = os.path.join(work_dir, "test_rust_harness.rs")
        with open(test_harness_rs, "w") as f:
            f.write("""extern crate reg_map;
use reg_map::*;

fn main() {
    let mut reg_val: u32 = 0;
    reg_val = CONTROL_REG::set_enable(reg_val, 1);
    assert_eq!(CONTROL_REG::get_enable(reg_val), 1);
    assert_eq!(reg_val, 1);

    reg_val = CONTROL_REG::set_mode(reg_val, 3);
    assert_eq!(CONTROL_REG::get_mode(reg_val), 3);
    assert_eq!(CONTROL_REG::get_enable(reg_val), 1); // verify preserved

    reg_val = CONTROL_REG::set_timeout(reg_val, 0x55);
    assert_eq!(CONTROL_REG::get_timeout(reg_val), 0x55);
    assert_eq!(CONTROL_REG::get_mode(reg_val), 3);
    assert_eq!(CONTROL_REG::get_enable(reg_val), 1);
}
""")
        rs_bin = os.path.join(work_dir, "test_rust_harness")
        run_command([rustc_bin, "--edition", "2021", "--extern", f"reg_map={rlib_out}", test_harness_rs, "-o", rs_bin])
        run_command([rs_bin])
        print("  ✓ Rust functional assertions executed successfully.")

    print("✓ Rust template verified successfully.\n")


# =============================================================================
# 5. Python Template Validator (Bring-up Driver)
# =============================================================================
def test_python(rmap_bin, work_dir):
    print("Testing 'python' template (Bring-up Driver)...")
    comp_out, spi_out, _ = export_examples("python", rmap_bin, work_dir)

    py_comp_file = os.path.join(comp_out, "python", "reg_map.py")
    py_spi_file = os.path.join(spi_out, "python", "reg_map.py")

    assert os.path.isfile(py_comp_file), f"Python output '{py_comp_file}' not found!"
    assert os.path.isfile(py_spi_file), f"Python output '{py_spi_file}' not found!"

    # Syntax check
    run_command([sys.executable, "-m", "py_compile", py_comp_file])
    run_command([sys.executable, "-m", "py_compile", py_spi_file])
    print("  ✓ py_compile syntax verification passed.")

    # Functional interactive tests via dynamic import
    spec = importlib.util.spec_from_file_location("reg_map_comp", py_comp_file)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)

    # Verify classes exist
    assert hasattr(mod, "Field")
    assert hasattr(mod, "Register")
    assert hasattr(mod, "CORE_SUBSYSTEMBlock")

    # Mock bus backend
    memory = {}
    def mock_read(addr):
        return memory.get(addr, 0)
    def mock_write(addr, val):
        memory[addr] = val

    block = mod.CORE_SUBSYSTEMBlock(base_addr=0x1000, read_fn=mock_read, write_fn=mock_write)
    assert hasattr(block, "control")
    assert hasattr(block, "status_flags")
    assert hasattr(block, "special_access")

    ctrl_reg = block.control
    assert ctrl_reg.offset == 0x0000
    assert "ENABLE" in ctrl_reg.fields
    assert "MODE" in ctrl_reg.fields
    assert "TIMEOUT" in ctrl_reg.fields

    # Test read/write register
    block.write_reg(ctrl_reg, 0x1234)
    assert block.read_reg(ctrl_reg) == 0x1234
    assert memory[0x1000] == 0x1234

    # Test field read/write (read-modify-write)
    block.write_field(ctrl_reg, "ENABLE", 1)
    assert block.read_field(ctrl_reg, "ENABLE") == 1

    block.write_field(ctrl_reg, "MODE", 7)
    assert block.read_field(ctrl_reg, "MODE") == 7
    assert block.read_field(ctrl_reg, "ENABLE") == 1

    # Verify missing callback exception
    bad_block = mod.CORE_SUBSYSTEMBlock(base_addr=0x0)
    try:
        bad_block.read_reg(ctrl_reg)
        assert False, "Expected RuntimeError for missing read callback"
    except RuntimeError:
        pass

    print("✓ Python template verified successfully.\n")


# =============================================================================
# 6. HTML Template Validator (Interactive Documentation)
# =============================================================================
def test_html(rmap_bin, work_dir):
    print("Testing 'html' template (Interactive HTML Documentation)...")
    comp_out, spi_out, _ = export_examples("html", rmap_bin, work_dir)

    html_comp_file = os.path.join(comp_out, "html", "reg_doc.html")
    html_spi_file = os.path.join(spi_out, "html", "reg_doc.html")

    assert os.path.isfile(html_comp_file), f"HTML output '{html_comp_file}' not found!"
    assert os.path.isfile(html_spi_file), f"HTML output '{html_spi_file}' not found!"

    for f_path in [html_comp_file, html_spi_file]:
        with open(f_path, "r", encoding="utf-8") as f:
            content = f.read()

        assert "<!DOCTYPE html>" in content
        assert "<html" in content
        assert "<head>" in content
        assert "<body" in content
        assert "</html>" in content

        # Verify no unrendered template tags remain
        assert "{{" not in content, f"Found unrendered Inja expression '{{{{' in {f_path}"
        assert "{%" not in content, f"Found unrendered Inja statement '{{%' in {f_path}"

        # HTML parsing
        parser = StrictHTMLParser()
        parser.feed(content)
        assert "table" in parser.tags, "Expected HTML tables in register documentation"
        assert "input" in parser.tags, "Expected interactive search input element"

    # Specific checks on comprehensive output
    with open(html_comp_file, "r", encoding="utf-8") as f:
        comp_content = f.read()
    assert "CORE_SUBSYSTEM" in comp_content
    assert "CONTROL" in comp_content
    assert "ENABLE" in comp_content
    assert "STATUS_FLAGS" in comp_content

    print("✓ HTML template verified successfully.\n")


# =============================================================================
# 7. Markdown Template Validator (Specification Tables)
# =============================================================================
def test_markdown(rmap_bin, work_dir):
    print("Testing 'markdown' template (Markdown Specification Tables)...")
    comp_out, spi_out, _ = export_examples("markdown", rmap_bin, work_dir)

    md_comp_file = os.path.join(comp_out, "markdown", "reg_doc.md")
    md_spi_file = os.path.join(spi_out, "markdown", "reg_doc.md")

    assert os.path.isfile(md_comp_file), f"Markdown output '{md_comp_file}' not found!"
    assert os.path.isfile(md_spi_file), f"Markdown output '{md_spi_file}' not found!"

    with open(md_comp_file, "r", encoding="utf-8") as f:
        content = f.read()

    # Structural assertions
    assert "# " in content and "Register Map Specification" in content
    assert "| Bits | Field Name | SW Access | HW Access | Reset | Description |" in content
    assert "| Block Name | Base Offset | Description |" in content
    assert "| `[0:0]` | **ENABLE** | `RW` | `RO` | `0x0001` | Core Enable |" in content
    assert "| `[3:1]` | **MODE** | `RW` | `RO` | `0x0002` | Operational Mode (3-bit binary reset) |" in content

    # Test with python-markdown if installed
    try:
        import markdown
        html = markdown.markdown(content, extensions=["tables"])
        assert len(html) > 100
        print("  ✓ Markdown package successfully rendered table HTML.")
    except ImportError:
        pass

    print("✓ Markdown template verified successfully.\n")


# =============================================================================
# 8. SystemRDL Template Validator (Accellera SystemRDL 2.0)
# =============================================================================
def test_systemrdl(rmap_bin, work_dir):
    print("Testing 'systemrdl' template (SystemRDL 2.0 Map Specification)...")
    comp_out, spi_out, _ = export_examples("systemrdl", rmap_bin, work_dir)

    rdl_comp_file = os.path.join(comp_out, "systemrdl", "reg_map.rdl")
    rdl_spi_file = os.path.join(spi_out, "systemrdl", "reg_map.rdl")

    assert os.path.isfile(rdl_comp_file), f"SystemRDL output '{rdl_comp_file}' not found!"
    assert os.path.isfile(rdl_spi_file), f"SystemRDL output '{rdl_spi_file}' not found!"

    with open(rdl_comp_file, "r", encoding="utf-8") as f:
        content = f.read()

    # Syntax and construct assertions
    assert re.search(r"addrmap\s+\w+\s*\{", content), "Missing addrmap component definition!"
    assert "default regwidth = 32;" in content
    assert "default sw = rw;" in content
    assert "default hw = r;" in content
    assert "regfile core_subsystem_rf {" in content
    assert "reg control_reg_t {" in content
    assert "field {" in content
    assert "sw = rw;" in content
    assert "hw = ro;" in content
    assert "enable[0:0]" in content
    assert "} control @ 0x0000;" in content

    # Check balanced braces
    open_braces = content.count("{")
    close_braces = content.count("}")
    assert open_braces == close_braces, f"Mismatched braces in SystemRDL: {open_braces} {{ vs {close_braces} }}"

    # Check peakrdl if installed
    peakrdl_bin = shutil.which("peakrdl")
    if peakrdl_bin:
        proc = subprocess.run([peakrdl_bin, "--help"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if proc.returncode == 0:
            print("  ✓ peakrdl CLI detected.")

    print("✓ SystemRDL template verified successfully.\n")


# =============================================================================
# 9. IP-XACT Template Validator (IEEE 1685-2014 XML)
# =============================================================================
def test_ipxact(rmap_bin, work_dir):
    print("Testing 'ipxact' template (IEEE 1685 IP-XACT XML)...")
    comp_out, spi_out, _ = export_examples("ipxact", rmap_bin, work_dir)

    xml_comp_file = os.path.join(comp_out, "ipxact", "reg_map.xml")
    xml_spi_file = os.path.join(spi_out, "ipxact", "reg_map.xml")

    assert os.path.isfile(xml_comp_file), f"IP-XACT output '{xml_comp_file}' not found!"
    assert os.path.isfile(xml_spi_file), f"IP-XACT output '{xml_spi_file}' not found!"

    for f_path in [xml_comp_file, xml_spi_file]:
        tree = ET.parse(f_path)
        root = tree.getroot()
        ns = {"ipxact": "http://www.accellera.org/XMLSchema/IPXACT/1685-2014"}
        assert "component" in root.tag

        vendor = root.find("ipxact:vendor", ns)
        assert vendor is not None and vendor.text == "rmap"

        name = root.find("ipxact:name", ns)
        assert name is not None and len(name.text) > 0

        # Memory maps
        mem_maps = root.find("ipxact:memoryMaps", ns)
        assert mem_maps is not None
        mmap = mem_maps.find("ipxact:memoryMap", ns)
        assert mmap is not None

        # Address blocks
        blocks = mmap.findall("ipxact:addressBlock", ns)
        assert len(blocks) > 0

        # Registers and fields
        for blk in blocks:
            regs = blk.findall("ipxact:register", ns)
            assert len(regs) > 0
            for reg in regs:
                assert reg.find("ipxact:name", ns) is not None
                assert reg.find("ipxact:addressOffset", ns) is not None
                fields = reg.findall("ipxact:field", ns)
                assert len(fields) > 0
                for fld in fields:
                    assert fld.find("ipxact:name", ns) is not None
                    assert fld.find("ipxact:bitOffset", ns) is not None
                    assert fld.find("ipxact:bitWidth", ns) is not None

    # xmllint if available
    xmllint_bin = shutil.which("xmllint")
    if xmllint_bin:
        run_command([xmllint_bin, "--noout", xml_comp_file])
        run_command([xmllint_bin, "--noout", xml_spi_file])
        print("  ✓ xmllint XML validation passed.")

    print("✓ IP-XACT template verified successfully.\n")


# =============================================================================
# 10. SVD Template Validator (ARM CMSIS-SVD XML)
# =============================================================================
def test_svd(rmap_bin, work_dir):
    print("Testing 'svd' template (ARM CMSIS-SVD XML)...")
    comp_out, spi_out, _ = export_examples("svd", rmap_bin, work_dir)

    svd_comp_file = os.path.join(comp_out, "svd", "reg_map.xml")
    svd_spi_file = os.path.join(spi_out, "svd", "reg_map.xml")

    assert os.path.isfile(svd_comp_file), f"SVD output '{svd_comp_file}' not found!"
    assert os.path.isfile(svd_spi_file), f"SVD output '{svd_spi_file}' not found!"

    for f_path in [svd_comp_file, svd_spi_file]:
        tree = ET.parse(f_path)
        root = tree.getroot()
        assert root.tag == "device"
        assert root.attrib.get("schemaVersion") == "1.3"

        assert root.find("vendor") is not None and root.find("vendor").text == "rmap"
        assert root.find("width") is not None
        assert root.find("addressUnitBits") is not None

        peripherals = root.find("peripherals")
        assert peripherals is not None
        periph_list = peripherals.findall("peripheral")
        assert len(periph_list) > 0

        for p in periph_list:
            assert p.find("name") is not None
            assert p.find("baseAddress") is not None
            regs_elem = p.find("registers")
            assert regs_elem is not None
            regs = regs_elem.findall("register")
            assert len(regs) > 0
            for r in regs:
                assert r.find("name") is not None
                assert r.find("addressOffset") is not None
                flds_elem = r.find("fields")
                assert flds_elem is not None
                flds = flds_elem.findall("field")
                assert len(flds) > 0
                for fld in flds:
                    assert fld.find("name") is not None
                    assert fld.find("bitOffset") is not None
                    assert fld.find("bitWidth") is not None

    # xmllint if available
    xmllint_bin = shutil.which("xmllint")
    if xmllint_bin:
        run_command([xmllint_bin, "--noout", svd_comp_file])
        run_command([xmllint_bin, "--noout", svd_spi_file])
        print("  ✓ xmllint CMSIS-SVD validation passed.")

    print("✓ SVD template verified successfully.\n")


# =============================================================================
# 11. JSON Template Validator (JSON Schema Export)
# =============================================================================
def test_json(rmap_bin, work_dir):
    print("Testing 'json' template (JSON Schema Export)...")
    comp_out, spi_out, _ = export_examples("json", rmap_bin, work_dir)

    json_comp_file = os.path.join(comp_out, "json", "reg_map.json")
    json_spi_file = os.path.join(spi_out, "json", "reg_map.json")

    assert os.path.isfile(json_comp_file), f"JSON output '{json_comp_file}' not found!"
    assert os.path.isfile(json_spi_file), f"JSON output '{json_spi_file}' not found!"

    for f_path in [json_comp_file, json_spi_file]:
        with open(f_path, "r", encoding="utf-8") as f:
            data = json.load(f)

        assert "name" in data and isinstance(data["name"], str)
        assert "reg_width" in data and isinstance(data["reg_width"], int)
        assert "blocks" in data and isinstance(data["blocks"], list)
        assert len(data["blocks"]) > 0

        for blk in data["blocks"]:
            assert "name" in blk
            assert "offset_hex" in blk
            assert "registers" in blk and isinstance(blk["registers"], list)
            assert len(blk["registers"]) > 0

            for reg in blk["registers"]:
                assert "name" in reg
                assert "offset_hex" in reg
                assert "offset_lsb" in reg and isinstance(reg["offset_lsb"], int)
                assert "size_width" in reg and isinstance(reg["size_width"], int)
                assert "access" in reg
                assert "fields" in reg and isinstance(reg["fields"], list)
                assert len(reg["fields"]) > 0

                for fld in reg["fields"]:
                    assert "name" in fld
                    assert "offset_lsb" in fld and isinstance(fld["offset_lsb"], int)
                    assert "size_width" in fld and isinstance(fld["size_width"], int)
                    assert "access" in fld
                    # Verify real boolean types (not string "true"/"false")
                    assert "is_rand" in fld and isinstance(fld["is_rand"], bool)
                    assert "volatile" in fld and isinstance(fld["volatile"], bool)
                    assert "has_reset" in fld and isinstance(fld["has_reset"], bool)

    # jq validation if available
    jq_bin = shutil.which("jq")
    if jq_bin:
        run_command([jq_bin, ".", json_comp_file])
        run_command([jq_bin, ".", json_spi_file])
        print("  ✓ jq JSON syntax validation passed.")

    print("✓ JSON template verified successfully.\n")


# =============================================================================
# 12. RTL Testbench Validator (Self-Checking SystemVerilog TB)
# =============================================================================
def test_rtl_tb(rmap_bin, work_dir):
    print("Testing 'rtl_tb' template (Self-Checking SystemVerilog TB)...")
    comp_out, spi_out, _ = export_examples("rtl_tb", rmap_bin, work_dir)

    tb_comp = os.path.join(comp_out, "sim", "tb_reg_map.sv")
    tb_spi = os.path.join(spi_out, "sim", "tb_reg_map.sv")

    assert os.path.isfile(tb_comp), f"RTL TB output '{tb_comp}' not found!"
    assert os.path.isfile(tb_spi), f"RTL TB output '{tb_spi}' not found!"

    with open(tb_comp, "r", encoding="utf-8") as f:
        content = f.read()

    assert "module tb_core_subsystem_reg_file;" in content
    assert "core_subsystem_reg_file #(" in content
    assert "task automatic bus_write(" in content
    assert "task automatic bus_read(" in content
    assert "task automatic check_val(" in content
    assert "Phase 1] Reset & Power-On Defaults" in content
    assert "Phase 2] Access Policy Verification" in content
    assert "Phase 3] Byte Strobe Masking Verification" in content
    assert "Phase 4] Hardware Sideband Interaction" in content

    # If iverilog is installed, compile and simulate
    iverilog_bin = shutil.which("iverilog")
    vvp_bin = shutil.which("vvp")
    if iverilog_bin and vvp_bin:
        rtl_file = os.path.join(comp_out, "rtl", "reg_map.sv")
        sim_vvp = os.path.join(work_dir, "sim_rtl.vvp")
        run_command([iverilog_bin, "-g2012", "-o", sim_vvp, f"-I{os.path.dirname(rtl_file)}", rtl_file, tb_comp])
        res = run_command([vvp_bin, sim_vvp])
        assert "ALL ASSERTIONS PASSED" in res.stdout
        print("  ✓ Icarus Verilog simulation executed and all assertions passed.")

    # If verilator is installed, run lint check
    verilator_bin = shutil.which("verilator")
    if verilator_bin:
        rtl_file = os.path.join(comp_out, "rtl", "reg_map.sv")
        run_command([verilator_bin, "--lint-only", "-Wno-fatal", "-Wno-DECLFILENAME", f"-I{os.path.dirname(rtl_file)}", rtl_file, tb_comp])
        print("  ✓ Verilator lint check passed on RTL testbench.")

    print("✓ RTL Testbench template verified successfully.\n")


# =============================================================================
# 13. pyuvm Testbench Validator (Open-Source Python UVM TB)
# =============================================================================
def test_pyuvm_tb(rmap_bin, work_dir):
    print("Testing 'pyuvm_tb' template (Open-Source Python UVM TB)...")
    comp_out, spi_out, _ = export_examples("pyuvm_tb", rmap_bin, work_dir)

    pyuvm_comp = os.path.join(comp_out, "sim", "tb_pyuvm.py")
    pyuvm_spi = os.path.join(spi_out, "sim", "tb_pyuvm.py")

    assert os.path.isfile(pyuvm_comp), f"pyuvm TB output '{pyuvm_comp}' not found!"
    assert os.path.isfile(pyuvm_spi), f"pyuvm TB output '{pyuvm_spi}' not found!"

    # Syntax check
    py_compile.compile(pyuvm_comp, doraise=True)
    py_compile.compile(pyuvm_spi, doraise=True)
    print("  ✓ py_compile syntax verification passed.")

    with open(pyuvm_comp, "r", encoding="utf-8") as f:
        content = f.read()

    assert "class RegBusItem(uvm_sequence_item):" in content
    assert "class RegBusDriver(uvm_driver):" in content
    assert "class RegBusMonitor(uvm_monitor):" in content
    assert "class RegBusAgent(uvm_agent):" in content
    assert "class RegEnv(uvm_env):" in content
    assert "class HwResetCheckSequence(uvm_sequence):" in content
    assert "class RegisterRwSequence(uvm_sequence):" in content
    assert "class PyUvmRegisterTest(uvm_test):" in content
    assert "@cocotb.test()" in content

    print("✓ pyuvm Testbench template verified successfully.\n")


# =============================================================================
# 14. UVM IEEE 1800.2 Testbench Validator (uvm-ieee VIP & Environment)
# =============================================================================
def test_uvm_tb(rmap_bin, work_dir):
    print("Testing 'uvm_tb' template (IEEE 1800.2 uvm-ieee Environment)...")
    comp_out, spi_out, _ = export_examples("uvm_tb", rmap_bin, work_dir)

    uvm_dir = os.path.join(comp_out, "sim", "uvm")
    bus_if = os.path.join(uvm_dir, "reg_bus_if.sv")
    bus_pkg = os.path.join(uvm_dir, "reg_bus_pkg.sv")
    env_sv = os.path.join(uvm_dir, "reg_env.sv")
    tests_sv = os.path.join(uvm_dir, "reg_tests.sv")
    top_sv = os.path.join(uvm_dir, "tb_top.sv")

    for fpath in [bus_if, bus_pkg, env_sv, tests_sv, top_sv]:
        assert os.path.isfile(fpath), f"UVM TB output '{fpath}' not found!"

    with open(bus_pkg, "r", encoding="utf-8") as f:
        pkg_content = f.read()
    assert "package reg_bus_pkg;" in pkg_content
    assert "class reg_bus_item extends uvm_sequence_item;" in pkg_content
    assert "class reg_bus_driver extends uvm_driver" in pkg_content
    assert "class reg_bus_monitor extends uvm_monitor;" in pkg_content
    assert "class reg_bus_agent extends uvm_agent;" in pkg_content
    assert "class reg_bus_adapter extends uvm_reg_adapter;" in pkg_content

    with open(tests_sv, "r", encoding="utf-8") as f:
        tests_content = f.read()
    assert "class core_subsystem_hw_reset_test extends core_subsystem_base_test;" in tests_content
    assert "class core_subsystem_bit_bash_test extends core_subsystem_base_test;" in tests_content
    assert "class core_subsystem_reg_access_test extends core_subsystem_base_test;" in tests_content

    with open(top_sv, "r", encoding="utf-8") as f:
        top_content = f.read()
    assert "module tb_top;" in top_content
    assert "reg_bus_if" in top_content
    assert "run_test();" in top_content

    print("✓ UVM IEEE 1800.2 Testbench template verified successfully.\n")


# =============================================================================
# 15. Simulation Makefile Validator
# =============================================================================
def test_sim_makefile(rmap_bin, work_dir):
    print("Testing 'sim_makefile' template (Multi-Tool Simulation Makefile)...")
    comp_out, spi_out, _ = export_examples("sim_makefile", rmap_bin, work_dir)

    mk_comp = os.path.join(comp_out, "sim", "Makefile")
    mk_spi = os.path.join(spi_out, "sim", "Makefile")

    assert os.path.isfile(mk_comp), f"Simulation Makefile '{mk_comp}' not found!"
    assert os.path.isfile(mk_spi), f"Simulation Makefile '{mk_spi}' not found!"

    with open(mk_comp, "r", encoding="utf-8") as f:
        content = f.read()

    assert "sim-rtl:" in content
    assert "sim-verilator:" in content
    assert "sim-pyuvm:" in content
    assert "sim-uvm:" in content
    assert "uvm-ieee" in content

    print("✓ Simulation Makefile template verified successfully.\n")


# =============================================================================
# Main Dispatcher
# =============================================================================
TEMPLATES = {
    "c": test_c,
    "rtl": test_rtl,
    "uvm": test_uvm,
    "rust": test_rust,
    "python": test_python,
    "html": test_html,
    "markdown": test_markdown,
    "systemrdl": test_systemrdl,
    "ipxact": test_ipxact,
    "svd": test_svd,
    "json": test_json,
    "rtl_tb": test_rtl_tb,
    "pyuvm_tb": test_pyuvm_tb,
    "uvm_tb": test_uvm_tb,
    "sim_makefile": test_sim_makefile,
}

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <template_name|all>")
        print(f"Supported templates: {', '.join(TEMPLATES.keys())}, all")
        sys.exit(1)

    target = sys.argv[1].lower()
    rmap_bin = find_rmap_binary()

    if target == "all":
        print(f"Running comprehensive tests for all {len(TEMPLATES)} templates...\n")
        failed = []
        for name, validator in TEMPLATES.items():
            work_dir = os.path.join(WORK_BASE, name)
            try:
                validator(rmap_bin, work_dir)
            except Exception as e:
                print(f"FAILED: Template '{name}' failed: {e}", file=sys.stderr)
                import traceback
                traceback.print_exc()
                failed.append(name)

        if failed:
            print(f"\n========================================", file=sys.stderr)
            print(f"FAILED: {len(failed)}/{len(TEMPLATES)} templates failed: {', '.join(failed)}", file=sys.stderr)
            print(f"========================================", file=sys.stderr)
            sys.exit(1)
        else:
            print("\n========================================")
            print(f"SUCCESS: All {len(TEMPLATES)} templates passed comprehensive validation!")
            print("========================================")
            sys.exit(0)

    elif target in TEMPLATES:
        work_dir = os.path.join(WORK_BASE, target)
        try:
            TEMPLATES[target](rmap_bin, work_dir)
            print(f"SUCCESS: Template '{target}' passed comprehensive validation.")
            sys.exit(0)
        except Exception as e:
            print(f"FAILED: Template '{target}' failed: {e}", file=sys.stderr)
            import traceback
            traceback.print_exc()
            sys.exit(1)
    else:
        print(f"Error: Unknown template '{target}'.", file=sys.stderr)
        print(f"Available templates: {', '.join(TEMPLATES.keys())}, all", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
