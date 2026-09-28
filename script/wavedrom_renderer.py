#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
wavedrom_renderer.py - Pure-Python WaveDrom to SVG and PNG timing diagram generator.

Renders standard WaveDrom JSON timing diagrams into robust, high-contrast SVG
and PNG images using portable vector primitives (lines, polygons, rects, text)
compatible with web browsers, Doxygen, and ImageMagick.
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
import xml.sax.saxutils as saxutils
from pathlib import Path


def wavedrom_to_svg(
    wave_json: dict,
    step_width: int = 44,
    signal_height: int = 24,
    theme: str = "dark"
) -> str:
    """
    Renders a standard WaveDrom dictionary to clean, standalone SVG.
    """
    signals = wave_json.get("signal", [])
    title = wave_json.get("head", {}).get("text", "")

    # Theme palette
    if theme == "light":
        c_bg = "#ffffff"
        c_grid = "#e0e0e0"
        c_title = "#0b4f6c"
        c_head = "#1a1a1a"
        c_wave = "#0288d1"
        c_clk = "#d84315"
        c_bus = "#00796b"
        c_bus_fill = "#e0f2f1"
        c_bus_text = "#004d40"
        c_cycle = "#757575"
        c_undef = "#9e9e9e"
    else:  # dark Solarized
        c_bg = "#002b36"
        c_grid = "#073642"
        c_title = "#2aa198"
        c_head = "#fdf6e3"
        c_wave = "#3498db"
        c_clk = "#f39c12"
        c_bus = "#2aa198"
        c_bus_fill = "#073642"
        c_bus_text = "#fdf6e3"
        c_cycle = "#657b83"
        c_undef = "#586e75"

    parsed_signals = []
    max_cycles = 0
    max_name_len = 0

    for s in signals:
        if not isinstance(s, dict):
            continue
        name = s.get("name", "")
        wave = s.get("wave", "")
        data = list(s.get("data", []))
        max_name_len = max(max_name_len, len(name))

        tokens = []
        data_idx = 0
        i = 0
        while i < len(wave):
            ch = wave[i]
            j = i + 1
            while j < len(wave) and wave[j] == '.':
                j += 1
            dur = j - i

            label = ""
            if ch in ('=', '2', '3', '4', '5'):
                if data_idx < len(data):
                    label = str(data[data_idx])
                    data_idx += 1

            tokens.append((ch, dur, label))
            i = j

        total_dur = sum(t[1] for t in tokens)
        max_cycles = max(max_cycles, total_dur)
        parsed_signals.append({
            "name": name,
            "tokens": tokens,
            "total_dur": total_dur
        })

    label_width = max(180, max_name_len * 9 + 30)
    wave_width = max_cycles * step_width
    total_width = label_width + wave_width + 40
    header_height = 50 if title else 30
    total_height = header_height + len(parsed_signals) * (signal_height + 20) + 20

    svg = []
    svg.append(f'<svg xmlns="http://www.w3.org/2000/svg" width="{total_width}" height="{total_height}" viewBox="0 0 {total_width} {total_height}">')
    svg.append(f'<rect width="{total_width}" height="{total_height}" fill="{c_bg}" rx="6" ry="6"/>')

    if title:
        svg.append(f'<text x="24" y="32" fill="{c_head}" font-family="-apple-system, BlinkMacSystemFont, Segoe UI, Roboto, sans-serif" font-size="15" font-weight="bold">{saxutils.escape(title)}</text>')

    start_x = label_width
    for c in range(max_cycles + 1):
        x = start_x + c * step_width
        svg.append(f'<line x1="{x}" y1="{header_height - 6}" x2="{x}" y2="{total_height - 15}" stroke="{c_grid}" stroke-dasharray="2,2" stroke-width="1"/>')
        if c < max_cycles:
            svg.append(f'<text x="{x + step_width // 2}" y="{header_height - 12}" fill="{c_cycle}" font-family="monospace" font-size="11" text-anchor="middle">{c}</text>')

    y = header_height + 10
    for sig in parsed_signals:
        name = sig["name"]
        tokens = sig["tokens"]

        y_top = y
        y_bot = y + signal_height
        y_mid = y + signal_height // 2

        svg.append(f'<text x="24" y="{y_mid + 4}" fill="{c_title}" font-family="-apple-system, BlinkMacSystemFont, Segoe UI, Roboto, monospace, sans-serif" font-size="13" font-weight="bold">{saxutils.escape(name)}</text>')

        cur_x = start_x
        prev_level = None

        for ch, dur, label in tokens:
            dur_px = dur * step_width
            next_x = cur_x + dur_px

            if ch in ('p', 'P'):
                for clk_idx in range(dur):
                    cx0 = cur_x + clk_idx * step_width
                    cx_mid = cx0 + step_width // 2
                    cx1 = cx0 + step_width
                    svg.append(f'<line x1="{cx0}" y1="{y_bot}" x2="{cx0}" y2="{y_top}" stroke="{c_clk}" stroke-width="2.5" stroke-linecap="round"/>')
                    svg.append(f'<line x1="{cx0}" y1="{y_top}" x2="{cx_mid}" y2="{y_top}" stroke="{c_clk}" stroke-width="2.5" stroke-linecap="round"/>')
                    svg.append(f'<line x1="{cx_mid}" y1="{y_top}" x2="{cx_mid}" y2="{y_bot}" stroke="{c_clk}" stroke-width="2.5" stroke-linecap="round"/>')
                    svg.append(f'<line x1="{cx_mid}" y1="{y_bot}" x2="{cx1}" y2="{y_bot}" stroke="{c_clk}" stroke-width="2.5" stroke-linecap="round"/>')
                    svg.append(f'<line x1="{cx0 - 3}" y1="{y_mid + 2}" x2="{cx0}" y2="{y_mid - 3}" stroke="{c_clk}" stroke-width="1.8"/>')
                    svg.append(f'<line x1="{cx0}" y1="{y_mid - 3}" x2="{cx0 + 3}" y2="{y_mid + 2}" stroke="{c_clk}" stroke-width="1.8"/>')
                prev_level = '0'
            elif ch == '0':
                if prev_level == '1':
                    svg.append(f'<line x1="{cur_x}" y1="{y_top}" x2="{cur_x}" y2="{y_bot}" stroke="{c_wave}" stroke-width="2.5" stroke-linecap="round"/>')
                svg.append(f'<line x1="{cur_x}" y1="{y_bot}" x2="{next_x}" y2="{y_bot}" stroke="{c_wave}" stroke-width="2.5" stroke-linecap="round"/>')
                prev_level = '0'
            elif ch == '1':
                if prev_level == '0':
                    svg.append(f'<line x1="{cur_x}" y1="{y_bot}" x2="{cur_x}" y2="{y_top}" stroke="{c_wave}" stroke-width="2.5" stroke-linecap="round"/>')
                svg.append(f'<line x1="{cur_x}" y1="{y_top}" x2="{next_x}" y2="{y_top}" stroke="{c_wave}" stroke-width="2.5" stroke-linecap="round"/>')
                prev_level = '1'
            elif ch in ('=', '2', '3', '4', '5'):
                b = 5
                poly = [
                    f"{cur_x + b},{y_top}",
                    f"{next_x - b},{y_top}",
                    f"{next_x},{y_mid}",
                    f"{next_x - b},{y_bot}",
                    f"{cur_x + b},{y_bot}",
                    f"{cur_x},{y_mid}"
                ]
                svg.append(f'<polygon points="{" ".join(poly)}" fill="{c_bus_fill}" stroke="{c_bus}" stroke-width="2"/>')
                if label:
                    cx = (cur_x + next_x) // 2
                    svg.append(f'<text x="{cx}" y="{y_mid + 4}" fill="{c_bus_text}" font-family="monospace, sans-serif" font-size="11" font-weight="bold" text-anchor="middle">{saxutils.escape(label)}</text>')
                prev_level = None
            elif ch == 'x':
                svg.append(f'<rect x="{cur_x}" y="{y_top + 1}" width="{dur_px}" height="{signal_height - 2}" fill="{c_undef}" fill-opacity="0.25"/>')
                svg.append(f'<line x1="{cur_x}" y1="{y_top + 1}" x2="{next_x}" y2="{y_top + 1}" stroke="{c_undef}" stroke-width="1.5"/>')
                svg.append(f'<line x1="{cur_x}" y1="{y_bot - 1}" x2="{next_x}" y2="{y_bot - 1}" stroke="{c_undef}" stroke-width="1.5"/>')
                svg.append(f'<line x1="{cur_x}" y1="{y_mid}" x2="{next_x}" y2="{y_mid}" stroke="{c_undef}" stroke-dasharray="2,2"/>')
                prev_level = None
            elif ch == 'z':
                svg.append(f'<line x1="{cur_x}" y1="{y_mid}" x2="{next_x}" y2="{y_mid}" stroke="{c_cycle}" stroke-width="2"/>')
                prev_level = None

            cur_x = next_x

        y += signal_height + 20

    svg.append('</svg>')
    return '\n'.join(svg)


def export_wavedrom(wave_json: dict, out_svg_path: str, out_png_path: str = "") -> bool:
    svg_content = wavedrom_to_svg(wave_json)
    os.makedirs(os.path.dirname(os.path.abspath(out_svg_path)), exist_ok=True)

    svg_changed = True
    if os.path.isfile(out_svg_path):
        try:
            with open(out_svg_path, "r", encoding="utf-8") as f:
                if f.read() == svg_content:
                    svg_changed = False
        except Exception:
            pass

    if svg_changed:
        with open(out_svg_path, "w", encoding="utf-8") as f:
            f.write(svg_content)

    if out_png_path:
        os.makedirs(os.path.dirname(os.path.abspath(out_png_path)), exist_ok=True)
        if not svg_changed and os.path.isfile(out_png_path) and os.path.getsize(out_png_path) > 0:
            return False

        convert_bin = shutil.which("convert")
        if convert_bin:
            res = subprocess.run([convert_bin, out_svg_path, out_png_path], capture_output=True)
            return res.returncode == 0
    return svg_changed


STANDARD_WAVEDROMS = {
    "rtl_sw_write_strobe": {
        "signal": [
            {"name": "clk_i", "wave": "p........"},
            {"name": "bus_addr_i", "wave": "x.=.=.x..", "data": ["0x00", "0x04"]},
            {"name": "bus_wdata_i", "wave": "x.=.=.x..", "data": ["0xCAFE", "0xBEEF"]},
            {"name": "bus_wstrb_i", "wave": "0.1.1.0.."},
            {"name": "bus_wr_en_i", "wave": "0.1.1.0.."},
            {"name": "sw_ctrl_wr_strobe_o", "wave": "0.1.0...."},
            {"name": "sw_status_wr_strobe_o", "wave": "0...1.0.."},
            {"name": "reg_ctrl_q", "wave": "x..=.....", "data": ["0xCAFE"]}
        ],
        "head": {"text": "RTL Template: Software Write Cycle with Byte Strobes & Pulse Strobe"}
    },
    "rtl_sw_read_strobe": {
        "signal": [
            {"name": "clk_i", "wave": "p........"},
            {"name": "bus_addr_i", "wave": "x.=.=.x..", "data": ["0x00", "0x04"]},
            {"name": "bus_rd_en_i", "wave": "0.1.1.0.."},
            {"name": "sw_ctrl_rd_strobe_o", "wave": "0.1.0...."},
            {"name": "sw_status_rd_strobe_o", "wave": "0...1.0.."},
            {"name": "bus_rdata_o", "wave": "x.=.=.x..", "data": ["0xCAFE", "0x0001"]}
        ],
        "head": {"text": "RTL Template: Software Read Cycle & Read Pulse Strobe"}
    },
    "rtl_hw_update": {
        "signal": [
            {"name": "clk_i", "wave": "p........"},
            {"name": "hw_status_err_i", "wave": "0...1.0.."},
            {"name": "hw_status_err_we_i", "wave": "0...1.0.."},
            {"name": "reg_status_q[0]", "wave": "0....1..."},
            {"name": "hw_status_err_o", "wave": "0....1..."}
        ],
        "head": {"text": "RTL Template: Hardware Update with Write-Enable Condition"}
    },
    "rtl_hw_arbitration": {
        "signal": [
            {"name": "clk_i", "wave": "p........"},
            {"name": "bus_wr_en_i (SW)", "wave": "0.1.0.1.0"},
            {"name": "bus_wdata_i", "wave": "x.=.x.=.x", "data": ["0xAA", "0xAA"]},
            {"name": "hw_data_we_i (HW)", "wave": "0.1.0.1.0"},
            {"name": "hw_data_i", "wave": "x.=.x.=.x", "data": ["0x55", "0x55"]},
            {"name": "reg_q (HW_PREC=1)", "wave": "x..=.....", "data": ["0x55 (HW)"]},
            {"name": "reg_q (HW_PREC=0)", "wave": "x......=.", "data": ["0xAA (SW)"]}
        ],
        "head": {"text": "RTL Template: Concurrent SW vs. HW Arbitration Precedence"}
    },
    "rtl_w1c_cycle": {
        "signal": [
            {"name": "clk_i", "wave": "p........"},
            {"name": "hw_irq_trigger_i", "wave": "0.1.0...."},
            {"name": "reg_irq_status_q", "wave": "0..1...0."},
            {"name": "bus_wr_en_i", "wave": "0....1.0."},
            {"name": "bus_wdata_i", "wave": "x....=.x.", "data": ["0x1 (W1C)"]},
            {"name": "sw_irq_wr_strobe_o", "wave": "0....1.0."}
        ],
        "head": {"text": "RTL Template: Write-1-to-Clear (W1C) Event Set & Software Clear"}
    },
    "apb_write_read_protocol": {
        "signal": [
            {"name": "PCLK", "wave": "p........"},
            {"name": "PADDR", "wave": "x.=.=.x..", "data": ["0x00", "0x04"]},
            {"name": "PWRITE", "wave": "0.1.0.0.."},
            {"name": "PSEL", "wave": "0.1.1.0.."},
            {"name": "PENABLE", "wave": "0.0.1.0.."},
            {"name": "PWDATA", "wave": "x.=.x....", "data": ["0x1234"]},
            {"name": "PSTRB", "wave": "x.=.x....", "data": ["0xF"]},
            {"name": "PREADY", "wave": "1........"},
            {"name": "PRDATA", "wave": "x...=.x..", "data": ["0x5678"]}
        ],
        "head": {"text": "APB4 Bridge Template: Setup and Access Phase Handshake"}
    },
    "axil_write_read_protocol": {
        "signal": [
            {"name": "ACLK", "wave": "p........"},
            {"name": "AWVALID", "wave": "0.1.0...."},
            {"name": "AWREADY", "wave": "0.1.0...."},
            {"name": "AWADDR", "wave": "x.=.x....", "data": ["0x00"]},
            {"name": "WVALID", "wave": "0.1.0...."},
            {"name": "WREADY", "wave": "0.1.0...."},
            {"name": "WDATA", "wave": "x.=.x....", "data": ["0xCAFE"]},
            {"name": "BVALID", "wave": "0..1.0..."},
            {"name": "BREADY", "wave": "1........"},
            {"name": "BRESP", "wave": "x..=.x...", "data": ["OKAY"]}
        ],
        "head": {"text": "AXI4-Lite Bridge Template: Write Address, Data & Response Channels"}
    },
    "sva_strobe_pulse": {
        "signal": [
            {"name": "clk_i", "wave": "p........"},
            {"name": "bus_wr_en_i", "wave": "0.1.0.1.1"},
            {"name": "sw_wr_strobe_o", "wave": "0.1.0.1.0"},
            {"name": "sva_pulse_assert", "wave": "1........"}
        ],
        "head": {"text": "SVA Template: Exactly-One-Cycle Software Strobe Pulse Assertion"}
    }
}


def render_all_standard_wavedroms(output_dir: str) -> list:
    os.makedirs(output_dir, exist_ok=True)
    generated = []
    for key, spec in STANDARD_WAVEDROMS.items():
        svg_file = os.path.join(output_dir, f"{key}.svg")
        png_file = os.path.join(output_dir, f"{key}.png")
        changed = export_wavedrom(spec, svg_file, png_file)
        generated.append((key, svg_file, png_file))
        if changed:
            print(f"  ✓ Rendered WaveDrom: {key}.svg & {key}.png")
        else:
            print(f"  - Kept WaveDrom (unchanged): {key}.svg & {key}.png")
    return generated


def main():
    parser = argparse.ArgumentParser(description="Render WaveDrom JSON to SVG/PNG")
    parser.add_argument("-i", "--input", help="Input WaveDrom JSON file")
    parser.add_argument("-o", "--output", help="Output SVG file path")
    parser.add_argument("-p", "--png", help="Optional output PNG file path")
    parser.add_argument("--all", action="store_true", help="Render all standard rmap template timing diagrams")
    parser.add_argument("--out-dir", default="docs/images/wavedrom", help="Output directory for --all")

    args = parser.parse_args()

    if args.all:
        print(f"--> Rendering all standard template WaveDrom timing diagrams into: {args.out_dir}")
        render_all_standard_wavedroms(args.out_dir)
        sys.exit(0)

    if args.input and args.output:
        with open(args.input, "r", encoding="utf-8") as f:
            data = json.load(f)
        export_wavedrom(data, args.output, args.png)
        print(f"✓ Rendered: {args.output}")
        sys.exit(0)

    parser.print_help()


if __name__ == "__main__":
    main()
