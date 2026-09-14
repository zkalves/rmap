#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
generate_coverage.py - Comprehensive Multi-Metric Code Coverage Analyzer for rmap

Extracts, aggregates, and reports all compiler code coverage metrics across C++ sources:
  1. Line Coverage (executable lines vs covered lines)
  2. Function Coverage (total functions vs executed functions)
  3. Branch Coverage (total branch paths vs branches taken)
  4. Condition Coverage / MC/DC (condition outcomes evaluated vs covered)
  5. Call Coverage (function call sites vs calls executed)
  6. Basic Block Coverage (total basic blocks vs executed basic blocks)

Outputs:
  - Formatted terminal table (--summary)
  - GitHub-Flavored Markdown report (--markdown <file>)
  - Machine-readable JSON report (--json <file>)
  - Interactive self-contained HTML report (--html <file>)
  - GitHub Action Step Summary (--github-step-summary)
  - README.md updater for the GitHub main page (--update-readme)
"""

import argparse
import gzip
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from collections import defaultdict, deque
from pathlib import Path
from typing import Any, Dict, List, Optional, Set, Tuple

PROJECT_ROOT = Path(__file__).resolve().parent.parent


def get_gcov_flags() -> List[str]:
    """Check gcov version and return supported flags."""
    flags = ["-b", "-c", "-f", "-a", "-u", "-j"]
    try:
        res = subprocess.run(["gcov", "--help"], capture_output=True, text=True)
        if "-g, --conditions" in res.stdout or "--conditions" in res.stdout:
            flags.append("-g")
    except Exception:
        pass
    return flags


def categorize_subsystem(rel_path: str) -> str:
    """Categorize source file into an architectural subsystem."""
    if rel_path.startswith("src/format/"):
        return "Format Parsers & Serializers"
    if any(k in rel_path for k in ["RegBitfieldBarWidget", "BlockMemoryMapWidget", "RegMapDelegate", "RegMapWindow"]):
        return "GUI Widgets & Main Window"
    if any(k in rel_path for k in ["RegMapTreeModel", "RegMapTreeItem", "RegMapTreeView", "UndoCommands", "Serializable", "SerializationContext", "ObjectFactory"]):
        return "Core Architecture & Model"
    if "CodeGenerator" in rel_path:
        return "Code Generation Engine"
    if any(k in rel_path for k in ["RegConfigWindow", "PreferencesWindow", "AboutWindow"]):
        return "Dialogs & Configuration"
    return "System Services & Utilities"


def parse_gcov_data(build_dir: Path, source_dir: Path, gcov_dir: Optional[Path] = None) -> Dict[str, Any]:
    """Run gcov on build artifacts and aggregate all 6 coverage metrics."""
    gcda_files = list(build_dir.glob("**/*.gcda"))
    if not gcda_files:
        print(f"Warning: No .gcda coverage data files found in {build_dir}. Have tests been executed?", file=sys.stderr)
        return {}

    gcov_flags = get_gcov_flags()
    all_files_data: Dict[str, Dict[str, Any]] = {}

    target_dir = gcov_dir if gcov_dir is not None else (PROJECT_ROOT / "work" / "coverage" / "gcov")
    target_dir.mkdir(parents=True, exist_ok=True)

    # Clean up stale .gcov* files from target_dir before running
    for old_file in target_dir.glob("*.gcov*"):
        try:
            old_file.unlink()
        except OSError:
            pass

    for gcda in sorted(gcda_files):
        obj = gcda.with_suffix(".o")
        if not obj.exists():
            continue

        if "src" not in gcda.parts:
            continue

        src_rel = Path(*gcda.parts[gcda.parts.index("src"):])
        src_str = str(src_rel)
        if src_str.endswith(".gcda"):
            src_str = src_str[:-5]
        src_cand = PROJECT_ROOT / src_str

        if not src_cand.exists():
            continue

        cmd = ["gcov"] + gcov_flags + ["-o", str(obj.resolve()), str(src_cand.resolve())]
        subprocess.run(cmd, cwd=target_dir, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    # Process all produced json.gz files
    for gz in target_dir.glob("*.gcov.json.gz"):
        try:
            with gzip.open(gz, "rt", encoding="utf-8") as f_in:
                data = json.load(f_in)
        except Exception:
            continue

        process_gcov_json(data, all_files_data)

    return all_files_data


_source_file_cache: Dict[str, Dict[str, Any]] = {}
CONTROL_FLOW_REGEX = re.compile(r"\b(if|else\s+if|while|for|switch|case|default)\b|\?")


def get_source_file_info(f_rel_str: str) -> Dict[str, Any]:
    """Parse source file to detect exclusion pragmas and synthetic compiler lines."""
    if f_rel_str in _source_file_cache:
        return _source_file_cache[f_rel_str]

    src_file = PROJECT_ROOT / f_rel_str
    if not src_file.exists():
        info = {"excluded_lines": set(), "excl_br_lines": set(), "synthetic_lines": set()}
        _source_file_cache[f_rel_str] = info
        return info

    lines = src_file.read_text(encoding="utf-8", errors="ignore").splitlines()
    excluded_lines = set()
    excl_br_lines = set()
    synthetic_lines = set()
    in_excl_block = False

    for idx, raw_l in enumerate(lines):
        ln = idx + 1
        lt = raw_l.strip()
        if "//" in lt:
            code_part = lt[:lt.index("//")].strip()
            comment_part = lt[lt.index("//"):].strip()
        else:
            code_part = lt
            comment_part = ""

        # LCOV / GCOV region markers
        if "LCOV_EXCL_START" in lt or "GCOV_EXCL_START" in lt:
            in_excl_block = True
            excluded_lines.add(ln)
            continue
        if "LCOV_EXCL_STOP" in lt or "GCOV_EXCL_STOP" in lt:
            in_excl_block = False
            excluded_lines.add(ln)
            continue
        if in_excl_block:
            excluded_lines.add(ln)
            continue

        # Single line exclusions
        if "LCOV_EXCL_LINE" in comment_part or "GCOV_EXCL_LINE" in comment_part:
            excluded_lines.add(ln)
            continue
        if "LCOV_EXCL_BR_LINE" in comment_part or "GCOV_EXCL_BR_LINE" in comment_part:
            excl_br_lines.add(ln)

        # Synthetic compiler lines (closing/opening braces, standalone new/delete allocations without control flow)
        if code_part in ["}", "{", "};", ")", ");", ""]:
            synthetic_lines.add(ln)
        elif ("new " in code_part or "delete " in code_part) and not CONTROL_FLOW_REGEX.search(code_part):
            synthetic_lines.add(ln)

    info = {
        "excluded_lines": excluded_lines,
        "excl_br_lines": excl_br_lines,
        "synthetic_lines": synthetic_lines,
    }
    _source_file_cache[f_rel_str] = info
    return info


def process_gcov_json(data: Dict[str, Any], all_files_data: Dict[str, Dict[str, Any]]) -> None:
    """Process single gcov JSON structure and aggregate metrics into all_files_data."""
    for f in data.get("files", []):
        f_path_str = f.get("file", "")
        if not f_path_str:
            continue
        f_path = Path(f_path_str).resolve()

        # Filter: must be inside source_dir and hand-written source
        try:
            f_rel = f_path.relative_to(PROJECT_ROOT)
        except ValueError:
            continue

        f_rel_str = str(f_rel).replace("\\", "/")
        if not f_rel_str.startswith("src/"):
            continue
        if any(bad in f_rel_str for bad in ["/proto/", "_autogen", "/work/", "/build/", "mocs_compilation"]):
            continue

        if f_rel_str not in all_files_data:
            all_files_data[f_rel_str] = {
                "file": f_rel_str,
                "subsystem": categorize_subsystem(f_rel_str),
                "lines": {},
                "funcs": {},
                "branches": {},
                "conds": [],
                "calls": [],
                "blocks_total": 0,
                "blocks_exec": 0,
            }

        entry = all_files_data[f_rel_str]

        # Map lines by function to resolve exception landing pad blocks in CFG
        fn_lines: Dict[str, List[Dict[str, Any]]] = defaultdict(list)
        for l in f.get("lines", []):
            fn_lines[l.get("function_name", "")].append(l)

        fn_landing_pads: Dict[str, Set[int]] = {}
        for fn_name, flist in fn_lines.items():
            throw_dsts = set()
            adj = defaultdict(set)
            all_source_blocks = set()

            for l in flist:
                for b in l.get("branches", []):
                    s = b.get("source_block_id")
                    d_blk = b.get("destination_block_id")
                    if s is not None and d_blk is not None:
                        adj[s].add(d_blk)
                        all_source_blocks.add(s)
                    if b.get("throw", False) and d_blk is not None and d_blk != 1:
                        throw_dsts.add(d_blk)

            landing_pads = {blk for blk in throw_dsts if isinstance(blk, int) and blk != 1}
            queue = deque(list(landing_pads))
            while queue:
                curr = queue.popleft()
                if not isinstance(curr, int) or curr == 1:
                    continue
                for nxt in adj.get(curr, set()):
                    if isinstance(nxt, int) and nxt not in landing_pads and nxt != 1:  # 1 is exit block
                        landing_pads.add(nxt)
                        queue.append(nxt)
                nxt_seq = curr + 1
                if nxt_seq not in landing_pads and nxt_seq != 1:
                    normal_predecessor = False
                    for s, dsts in adj.items():
                        if s not in landing_pads and nxt_seq in dsts:
                            normal_predecessor = True
                            break
                    if not normal_predecessor and nxt_seq in all_source_blocks:
                        landing_pads.add(nxt_seq)
                        queue.append(nxt_seq)

            fn_landing_pads[fn_name] = landing_pads

        src_info = get_source_file_info(f_rel_str)
        excl_lines = src_info["excluded_lines"]
        excl_br = src_info["excl_br_lines"]
        synth_lines = src_info["synthetic_lines"]

        # Aggregate Lines
        for l in f.get("lines", []):
            ln = l.get("line_number", 0)
            if ln in excl_lines:
                continue
            cnt = l.get("count", 0)
            entry["lines"][ln] = entry["lines"].get(ln, 0) + cnt
            fn_name = l.get("function_name", "")
            lp = fn_landing_pads.get(fn_name, set())

            is_synth = ln in synth_lines
            is_excl_br = ln in excl_br or ln in excl_lines

            # Aggregate Branches on this line
            for bi, b in enumerate(l.get("branches", [])):
                b_key = (ln, bi)
                cnt_br = b.get("count", 0)
                is_throw = bool(b.get("throw", False))
                s_blk = b.get("source_block_id")
                is_landing = bool(s_blk is not None and s_blk in lp)
                if b_key not in entry["branches"]:
                    entry["branches"][b_key] = {
                        "count": cnt_br,
                        "throw": is_throw,
                        "landing_pad": is_landing,
                        "synthetic": is_synth,
                        "excluded": is_excl_br,
                    }
                else:
                    if isinstance(entry["branches"][b_key], dict):
                        entry["branches"][b_key]["count"] += cnt_br
                        if is_landing:
                            entry["branches"][b_key]["landing_pad"] = True
                        if is_synth:
                            entry["branches"][b_key]["synthetic"] = True
                        if is_excl_br:
                            entry["branches"][b_key]["excluded"] = True
                    else:
                        entry["branches"][b_key] = {
                            "count": entry["branches"][b_key] + cnt_br,
                            "throw": is_throw,
                            "landing_pad": is_landing,
                            "synthetic": is_synth,
                            "excluded": is_excl_br,
                        }

            # Aggregate Conditions
            unexec_b = [b for b in l.get("branches", []) if b.get("count", 0) == 0]
            line_is_landing = bool(unexec_b and all(b.get("throw", False) or (b.get("source_block_id") is not None and b.get("source_block_id") in lp) for b in unexec_b))
            for c in l.get("conditions", []):
                c_dict = dict(c)
                c_dict["line_number"] = ln
                if line_is_landing and c.get("covered", 0) == 0:
                    c_dict["landing_pad"] = True
                if is_synth:
                    c_dict["synthetic"] = True
                if is_excl_br:
                    c_dict["excluded"] = True
                entry["conds"].append(c_dict)

            # Aggregate Calls
            if not is_excl_br:
                entry["calls"].extend(l.get("calls", []))

        # Aggregate Functions & Blocks
        for fn in f.get("functions", []):
            name = fn.get("demangled_name") or fn.get("name", "unknown")
            exec_cnt = fn.get("execution_count", 0)
            entry["funcs"][name] = entry["funcs"].get(name, 0) + exec_cnt
            entry["blocks_total"] += fn.get("blocks", 0)
            entry["blocks_exec"] += fn.get("blocks_executed", 0)

    return all_files_data


def compute_metrics(all_files_data: Dict[str, Dict[str, Any]], exclude_throw_branches: bool = True) -> Dict[str, Any]:
    """Compute summary statistics for all 6 coverage metrics."""
    file_stats = []

    tot_lines_exec = tot_lines = 0
    tot_funcs_exec = tot_funcs = 0
    tot_branches_exec = tot_branches = 0
    tot_branches_raw_exec = tot_branches_raw = 0
    tot_conds_exec = tot_conds = 0
    tot_calls_exec = tot_calls = 0
    tot_blocks_exec = tot_blocks = 0

    subsystem_map: Dict[str, Dict[str, int]] = {}

    for path, d in sorted(all_files_data.items()):
        l_tot = len(d["lines"])
        if l_tot == 0:
            continue
        l_cov = sum(1 for c in d["lines"].values() if c > 0)

        fn_tot = len(d["funcs"])
        fn_cov = sum(1 for c in d["funcs"].values() if c > 0)

        br_tot = 0
        br_cov = 0
        raw_br_tot = 0
        raw_br_cov = 0

        for b_val in d["branches"].values():
            if isinstance(b_val, dict):
                cnt = b_val.get("count", 0)
                is_throw = b_val.get("throw", False)
                is_landing = b_val.get("landing_pad", False)
                is_synth = b_val.get("synthetic", False)
                is_excl = b_val.get("excluded", False)
            else:
                cnt = b_val
                is_throw = False
                is_landing = False
                is_synth = False
                is_excl = False

            raw_br_tot += 1
            if cnt > 0:
                raw_br_cov += 1

            if exclude_throw_branches and (is_throw or is_landing or is_synth or is_excl):
                continue

            br_tot += 1
            if cnt > 0:
                br_cov += 1

        cd_tot = sum(
            c.get("count", 0)
            for c in d["conds"]
            if not (exclude_throw_branches and (c.get("landing_pad", False) or c.get("synthetic", False) or c.get("excluded", False)))
        )
        cd_cov = sum(
            c.get("covered", 0)
            for c in d["conds"]
            if not (exclude_throw_branches and (c.get("landing_pad", False) or c.get("synthetic", False) or c.get("excluded", False)))
        )

        cl_tot = len(d["calls"])
        cl_cov = sum(1 for c in d["calls"] if c.get("returned", 0) > 0)

        blk_tot = d["blocks_total"]
        blk_cov = d["blocks_exec"]

        tot_lines += l_tot
        tot_lines_exec += l_cov
        tot_funcs += fn_tot
        tot_funcs_exec += fn_cov
        tot_branches += br_tot
        tot_branches_exec += br_cov
        tot_branches_raw += raw_br_tot
        tot_branches_raw_exec += raw_br_cov
        tot_conds += cd_tot
        tot_conds_exec += cd_cov
        tot_calls += cl_tot
        tot_calls_exec += cl_cov
        tot_blocks += blk_tot
        tot_blocks_exec += blk_cov

        sub = d["subsystem"]
        if sub not in subsystem_map:
            subsystem_map[sub] = {
                "lines_tot": 0, "lines_cov": 0,
                "funcs_tot": 0, "funcs_cov": 0,
                "branches_tot": 0, "branches_cov": 0,
                "branches_raw_tot": 0, "branches_raw_cov": 0,
                "conds_tot": 0, "conds_cov": 0,
                "calls_tot": 0, "calls_cov": 0,
                "blocks_tot": 0, "blocks_cov": 0,
            }
        sm = subsystem_map[sub]
        sm["lines_tot"] += l_tot
        sm["lines_cov"] += l_cov
        sm["funcs_tot"] += fn_tot
        sm["funcs_cov"] += fn_cov
        sm["branches_tot"] += br_tot
        sm["branches_cov"] += br_cov
        sm["branches_raw_tot"] += raw_br_tot
        sm["branches_raw_cov"] += raw_br_cov
        sm["conds_tot"] += cd_tot
        sm["conds_cov"] += cd_cov
        sm["calls_tot"] += cl_tot
        sm["calls_cov"] += cl_cov
        sm["blocks_tot"] += blk_tot
        sm["blocks_cov"] += blk_cov

        file_stats.append({
            "file": path,
            "subsystem": sub,
            "lines": {"total": l_tot, "covered": l_cov, "percent": round((l_cov / l_tot * 100) if l_tot else 0.0, 2)},
            "functions": {"total": fn_tot, "covered": fn_cov, "percent": round((fn_cov / fn_tot * 100) if fn_tot else 0.0, 2)},
            "branches": {"total": br_tot, "covered": br_cov, "percent": round((br_cov / br_tot * 100) if br_tot else 0.0, 2)},
            "branches_raw": {"total": raw_br_tot, "covered": raw_br_cov, "percent": round((raw_br_cov / raw_br_tot * 100) if raw_br_tot else 0.0, 2)},
            "conditions": {"total": cd_tot, "covered": cd_cov, "percent": round((cd_cov / cd_tot * 100) if cd_tot else 0.0, 2)},
            "calls": {"total": cl_tot, "covered": cl_cov, "percent": round((cl_cov / cl_tot * 100) if cl_tot else 0.0, 2)},
            "blocks": {"total": blk_tot, "covered": blk_cov, "percent": round((blk_cov / blk_tot * 100) if blk_tot else 0.0, 2)},
        })

    subsystems_list = []
    for sub_name, sm in sorted(subsystem_map.items()):
        subsystems_list.append({
            "name": sub_name,
            "lines": {"total": sm["lines_tot"], "covered": sm["lines_cov"], "percent": round((sm["lines_cov"] / sm["lines_tot"] * 100) if sm["lines_tot"] else 0.0, 2)},
            "functions": {"total": sm["funcs_tot"], "covered": sm["funcs_cov"], "percent": round((sm["funcs_cov"] / sm["funcs_tot"] * 100) if sm["funcs_tot"] else 0.0, 2)},
            "branches": {"total": sm["branches_tot"], "covered": sm["branches_cov"], "percent": round((sm["branches_cov"] / sm["branches_tot"] * 100) if sm["branches_tot"] else 0.0, 2)},
            "branches_raw": {"total": sm["branches_raw_tot"], "covered": sm["branches_raw_cov"], "percent": round((sm["branches_raw_cov"] / sm["branches_raw_tot"] * 100) if sm["branches_raw_tot"] else 0.0, 2)},
            "conditions": {"total": sm["conds_tot"], "covered": sm["conds_cov"], "percent": round((sm["conds_cov"] / sm["conds_tot"] * 100) if sm["conds_tot"] else 0.0, 2)},
            "calls": {"total": sm["calls_tot"], "covered": sm["calls_cov"], "percent": round((sm["calls_cov"] / sm["calls_tot"] * 100) if sm["calls_tot"] else 0.0, 2)},
            "blocks": {"total": sm["blocks_tot"], "covered": sm["blocks_cov"], "percent": round((sm["blocks_cov"] / sm["blocks_tot"] * 100) if sm["blocks_tot"] else 0.0, 2)},
        })

    summary = {
        "lines": {"total": tot_lines, "covered": tot_lines_exec, "percent": round((tot_lines_exec / tot_lines * 100) if tot_lines else 0.0, 2)},
        "functions": {"total": tot_funcs, "covered": tot_funcs_exec, "percent": round((tot_funcs_exec / tot_funcs * 100) if tot_funcs else 0.0, 2)},
        "branches": {"total": tot_branches, "covered": tot_branches_exec, "percent": round((tot_branches_exec / tot_branches * 100) if tot_branches else 0.0, 2)},
        "branches_raw": {"total": tot_branches_raw, "covered": tot_branches_raw_exec, "percent": round((tot_branches_raw_exec / tot_branches_raw * 100) if tot_branches_raw else 0.0, 2)},
        "conditions": {"total": tot_conds, "covered": tot_conds_exec, "percent": round((tot_conds_exec / tot_conds * 100) if tot_conds else 0.0, 2)},
        "calls": {"total": tot_calls, "covered": tot_calls_exec, "percent": round((tot_calls_exec / tot_calls * 100) if tot_calls else 0.0, 2)},
        "blocks": {"total": tot_blocks, "covered": tot_blocks_exec, "percent": round((tot_blocks_exec / tot_blocks * 100) if tot_blocks else 0.0, 2)},
    }

    return {
        "summary": summary,
        "subsystems": subsystems_list,
        "files": file_stats,
    }


def get_color_for_percent(pct: float) -> str:
    """Return badge/progress color based on coverage percentage."""
    if pct >= 80.0:
        return "brightgreen"
    elif pct >= 65.0:
        return "green"
    elif pct >= 50.0:
        return "yellow"
    elif pct >= 35.0:
        return "orange"
    else:
        return "red"


def render_console_summary(metrics: Dict[str, Any]) -> str:
    """Render a clean ASCII/ANSI summary table for the terminal."""
    s = metrics["summary"]
    lines = [
        "",
        "==========================================================================================",
        "                                 rmap CODE COVERAGE REPORT                                ",
        "==========================================================================================",
        f"  {'Metric':<26} | {'Covered':>9} | {'Total':>9} | {'Coverage Rate':>14} | {'Status':>8}",
        "------------------------------------------------------------------------------------------",
    ]

    metrics_order = [
        ("Lines", s["lines"]),
        ("Functions", s["functions"]),
        ("Branches (Decision)", s["branches"]),
        ("Conditions (MC/DC)", s["conditions"]),
        ("Calls", s["calls"]),
        ("Basic Blocks", s["blocks"]),
    ]
    if "branches_raw" in s and s["branches_raw"]["total"] != s["branches"]["total"]:
        metrics_order.append(("  ↳ Raw (w/ Unwind)", s["branches_raw"]))

    for name, data in metrics_order:
        pct = data["percent"]
        status = "PASSED" if pct >= 50.0 else "WARN"
        lines.append(f"  {name:<26} | {data['covered']:>9d} | {data['total']:>9d} | {pct:>13.2f}% | {status:>8}")

    lines.append("==========================================================================================")
    lines.append("  Subsystems Breakdown:")
    lines.append("------------------------------------------------------------------------------------------")
    lines.append(f"  {'Subsystem':<32} | {'Lines':>9} | {'Funcs':>9} | {'Branch':>9} | {'Cond':>9}")
    lines.append("------------------------------------------------------------------------------------------")

    for sub in metrics["subsystems"]:
        lines.append(
            f"  {sub['name']:<32} | {sub['lines']['percent']:>8.1f}% | {sub['functions']['percent']:>8.1f}% | "
            f"{sub['branches']['percent']:>8.1f}% | {sub['conditions']['percent']:>8.1f}%"
        )
    lines.append("==========================================================================================")
    lines.append("")
    return "\n".join(lines)


def render_markdown_report(metrics: Dict[str, Any]) -> str:
    """Generate GitHub-Flavored Markdown report with tables and badges."""
    s = metrics["summary"]
    md = []

    md.append("## Code Coverage Metrics")
    md.append("")
    md.append("Automated test coverage analysis across all 6 compiler-supported metrics:")
    md.append("")
    md.append("| Metric | Covered | Total | Coverage Rate | Status |")
    md.append("| :--- | :---: | :---: | :---: | :---: |")

    metrics_rows = [
        ("**Lines**", s["lines"]),
        ("**Functions**", s["functions"]),
        ("**Branches (Decision)**", s["branches"]),
        ("**Conditions (MC/DC)**", s["conditions"]),
        ("**Calls**", s["calls"]),
        ("**Basic Blocks**", s["blocks"]),
    ]

    for label, data in metrics_rows:
        pct = data["percent"]
        icon = "✅" if pct >= 50.0 else "⚠️"
        md.append(f"| {label} | {data['covered']:,} | {data['total']:,} | **{pct:.2f}%** | {icon} |")

    if "branches_raw" in s and s["branches_raw"]["total"] != s["branches"]["total"]:
        raw = s["branches_raw"]
        md.append(f"| *Branches (Raw w/ Unwind)* | {raw['covered']:,} | {raw['total']:,} | *{raw['percent']:.2f}%* | ℹ️ |")

    md.append("")
    md.append("> [!NOTE]")
    md.append("> **Branch & Condition Coverage Measurement**: In accordance with DO-178C, ISO 26262, and `gcovr` standards, decision branch coverage tracks actual logical control branches (`if`, `switch`, `while`, ternary). Compiler-synthesized exception unwinding landing pads (`throw: true`), allocation checks (`new`/`delete`), and destructor cleanups are excluded from decision branches and shown transparently in raw metrics. Standard exclusion pragmas (`// GCOV_EXCL_LINE`, `// LCOV_EXCL_START`/`STOP`, `// GCOV_EXCL_BR_LINE`) are honored.")
    md.append("")
    md.append("### Architectural Subsystems Breakdown")
    md.append("")
    md.append("| Subsystem | Lines (%) | Functions (%) | Branches (%) | Conditions (%) | Calls (%) | Blocks (%) |")
    md.append("| :--- | :---: | :---: | :---: | :---: | :---: | :---: |")

    for sub in metrics["subsystems"]:
        md.append(
            f"| **{sub['name']}** | {sub['lines']['percent']:.1f}% ({sub['lines']['covered']}/{sub['lines']['total']}) | "
            f"{sub['functions']['percent']:.1f}% ({sub['functions']['covered']}/{sub['functions']['total']}) | "
            f"{sub['branches']['percent']:.1f}% ({sub['branches']['covered']}/{sub['branches']['total']}) | "
            f"{sub['conditions']['percent']:.1f}% ({sub['conditions']['covered']}/{sub['conditions']['total']}) | "
            f"{sub['calls']['percent']:.1f}% | {sub['blocks']['percent']:.1f}% |"
        )

    md.append("")
    md.append("<details>")
    md.append("<summary><b>Detailed Source Files Coverage (Click to expand)</b></summary>")
    md.append("")
    md.append("| Source File | Subsystem | Lines (%) | Functions (%) | Branches (%) | Conditions (%) | Calls (%) | Blocks (%) |")
    md.append("| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |")

    for f in metrics["files"]:
        md.append(
            f"| `{f['file']}` | {f['subsystem']} | {f['lines']['percent']:.1f}% | {f['functions']['percent']:.1f}% | "
            f"{f['branches']['percent']:.1f}% | {f['conditions']['percent']:.1f}% | {f['calls']['percent']:.1f}% | {f['blocks']['percent']:.1f}% |"
        )

    md.append("")
    md.append("</details>")
    md.append("")
    md.append("### Running Coverage Locally")
    md.append("")
    md.append("```bash")
    md.append("# Run unit tests with coverage instrumentation and output summary report")
    md.append("make coverage")
    md.append("")
    md.append("# Generate HTML and Markdown coverage reports")
    md.append("make coverage-report")
    md.append("```")
    md.append("")
    return "\n".join(md)


def render_html_report(metrics: Dict[str, Any]) -> str:
    """Generate self-contained, responsive HTML coverage report."""
    s = metrics["summary"]

    html_cards = []
    card_configs = [
        ("Line Coverage", s["lines"], "#2aa198"),
        ("Function Coverage", s["functions"], "#268bd2"),
        ("Branch Coverage", s["branches"], "#859900"),
        ("Condition (MC/DC)", s["conditions"], "#b58900"),
        ("Call Coverage", s["calls"], "#6c71c4"),
        ("Block Coverage", s["blocks"], "#d33682"),
    ]

    for title, data, color in card_configs:
        pct = data["percent"]
        html_cards.append(f"""
        <div class="card">
            <div class="card-title">{title}</div>
            <div class="card-value" style="color: {color};">{pct:.1f}%</div>
            <div class="progress-bar-bg">
                <div class="progress-bar-fill" style="width: {min(pct, 100):.1f}%; background-color: {color};"></div>
            </div>
            <div class="card-meta">{data['covered']:,} of {data['total']:,} covered</div>
        </div>
        """)

    sub_rows = []
    for sub in metrics["subsystems"]:
        sub_rows.append(f"""
        <tr>
            <td><strong>{sub['name']}</strong></td>
            <td>{sub['lines']['percent']:.1f}% <span class="sub-cnt">({sub['lines']['covered']}/{sub['lines']['total']})</span></td>
            <td>{sub['functions']['percent']:.1f}% <span class="sub-cnt">({sub['functions']['covered']}/{sub['functions']['total']})</span></td>
            <td>{sub['branches']['percent']:.1f}% <span class="sub-cnt">({sub['branches']['covered']}/{sub['branches']['total']})</span></td>
            <td>{sub['conditions']['percent']:.1f}% <span class="sub-cnt">({sub['conditions']['covered']}/{sub['conditions']['total']})</span></td>
            <td>{sub['calls']['percent']:.1f}%</td>
            <td>{sub['blocks']['percent']:.1f}%</td>
        </tr>
        """)

    file_rows = []
    for f in metrics["files"]:
        lp = f['lines']['percent']
        cls = "high" if lp >= 80 else ("med" if lp >= 50 else "low")
        file_rows.append(f"""
        <tr>
            <td><code>{f['file']}</code></td>
            <td>{f['subsystem']}</td>
            <td class="rate {cls}">{lp:.1f}% <span class="sub-cnt">({f['lines']['covered']}/{f['lines']['total']})</span></td>
            <td class="rate">{f['functions']['percent']:.1f}%</td>
            <td class="rate">{f['branches']['percent']:.1f}%</td>
            <td class="rate">{f['conditions']['percent']:.1f}%</td>
            <td class="rate">{f['calls']['percent']:.1f}%</td>
            <td class="rate">{f['blocks']['percent']:.1f}%</td>
        </tr>
        """)

    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>rmap — Code Coverage Report</title>
    <style>
        :root {{
            --bg: #002b36;
            --surface: #073642;
            --surface2: #0e4250;
            --text: #93a1a1;
            --text-strong: #fdf6e3;
            --border: #586e75;
            --green: #859900;
            --cyan: #2aa198;
            --blue: #268bd2;
            --yellow: #b58900;
            --red: #dc322f;
        }}
        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background-color: var(--bg);
            color: var(--text);
            margin: 0;
            padding: 30px;
        }}
        .container {{
            max-width: 1200px;
            margin: 0 auto;
        }}
        header {{
            margin-bottom: 30px;
            border-bottom: 1px solid var(--surface2);
            padding-bottom: 20px;
        }}
        h1 {{
            color: var(--text-strong);
            margin: 0 0 10px 0;
            font-size: 28px;
        }}
        p.subtitle {{
            margin: 0;
            color: var(--text);
            font-size: 14px;
        }}
        .grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
            gap: 15px;
            margin-bottom: 35px;
        }}
        .card {{
            background: var(--surface);
            border: 1px solid var(--surface2);
            border-radius: 8px;
            padding: 16px;
            box-shadow: 0 2px 6px rgba(0,0,0,0.25);
        }}
        .card-title {{
            font-size: 12px;
            text-transform: uppercase;
            letter-spacing: 0.05em;
            color: var(--text);
            margin-bottom: 8px;
        }}
        .card-value {{
            font-size: 32px;
            font-weight: 700;
            margin-bottom: 8px;
        }}
        .progress-bar-bg {{
            height: 6px;
            background: rgba(255,255,255,0.1);
            border-radius: 3px;
            overflow: hidden;
            margin-bottom: 8px;
        }}
        .progress-bar-fill {{
            height: 100%;
            border-radius: 3px;
        }}
        .card-meta {{
            font-size: 11px;
            color: var(--text);
        }}
        h2 {{
            color: var(--text-strong);
            font-size: 20px;
            margin-top: 30px;
            margin-bottom: 15px;
        }}
        table {{
            width: 100%;
            border-collapse: collapse;
            background: var(--surface);
            border-radius: 8px;
            overflow: hidden;
            margin-bottom: 30px;
            box-shadow: 0 2px 6px rgba(0,0,0,0.25);
        }}
        th, td {{
            padding: 12px 14px;
            text-align: left;
            border-bottom: 1px solid var(--surface2);
            font-size: 13px;
        }}
        th {{
            background: var(--surface2);
            color: var(--text-strong);
            font-weight: 600;
        }}
        tr:hover {{
            background: rgba(255,255,255,0.03);
        }}
        .rate {{
            font-weight: 600;
        }}
        .rate.high {{ color: var(--green); }}
        .rate.med {{ color: var(--yellow); }}
        .rate.low {{ color: var(--red); }}
        .sub-cnt {{
            font-size: 11px;
            color: var(--text);
            font-weight: normal;
        }}
        code {{
            font-family: ui-monospace, SFMono-Regular, Consolas, monospace;
            color: var(--cyan);
            font-size: 12px;
        }}
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>rmap — Code Coverage Report</h1>
            <p class="subtitle">Complete multi-metric validation across 6 compiler coverage metrics</p>
        </header>

        <div class="grid">
            {''.join(html_cards)}
        </div>

        <h2>Architectural Subsystems</h2>
        <table>
            <thead>
                <tr>
                    <th>Subsystem</th>
                    <th>Lines</th>
                    <th>Functions</th>
                    <th>Branches</th>
                    <th>Conditions (MC/DC)</th>
                    <th>Calls</th>
                    <th>Blocks</th>
                </tr>
            </thead>
            <tbody>
                {''.join(sub_rows)}
            </tbody>
        </table>

        <h2>Source Files Coverage</h2>
        <table>
            <thead>
                <tr>
                    <th>File</th>
                    <th>Subsystem</th>
                    <th>Lines</th>
                    <th>Functions</th>
                    <th>Branches</th>
                    <th>Conditions</th>
                    <th>Calls</th>
                    <th>Blocks</th>
                </tr>
            </thead>
            <tbody>
                {''.join(file_rows)}
            </tbody>
        </table>
    </div>
</body>
</html>"""
    return html


def update_readme(readme_path: Path, metrics: Dict[str, Any]) -> None:
    """Update README.md with coverage badges and the full Code Coverage Metrics section."""
    if not readme_path.exists():
        print(f"Warning: {readme_path} not found for updating.", file=sys.stderr)
        return

    content = readme_path.read_text(encoding="utf-8")
    s = metrics["summary"]

    # 1. Prepare Coverage Badges
    l_pct = s["lines"]["percent"]
    f_pct = s["functions"]["percent"]
    b_pct = s["branches"]["percent"]
    c_pct = s["conditions"]["percent"]

    l_col = get_color_for_percent(l_pct)
    f_col = get_color_for_percent(f_pct)
    b_col = get_color_for_percent(b_pct)
    c_col = get_color_for_percent(c_pct)

    cov_badges = (
        f"[![Line Coverage](https://img.shields.io/badge/Line_Coverage-{l_pct:.1f}%25-{l_col}.svg)](#code-coverage-metrics)\n"
        f"[![Function Coverage](https://img.shields.io/badge/Function_Coverage-{f_pct:.1f}%25-{f_col}.svg)](#code-coverage-metrics)\n"
        f"[![Branch Coverage](https://img.shields.io/badge/Branch_Coverage-{b_pct:.1f}%25-{b_col}.svg)](#code-coverage-metrics)\n"
        f"[![Condition Coverage](https://img.shields.io/badge/Condition_Coverage-{c_pct:.1f}%25-{c_col}.svg)](#code-coverage-metrics)"
    )

    # Insert or replace coverage badges below existing badges in README.md
    badge_marker_start = "<!-- COVERAGE_BADGES_START -->"
    badge_marker_end = "<!-- COVERAGE_BADGES_END -->"

    if badge_marker_start in content and badge_marker_end in content:
        pattern = re.compile(rf"{re.escape(badge_marker_start)}.*?{re.escape(badge_marker_end)}", re.DOTALL)
        content = pattern.sub(f"{badge_marker_start}\n{cov_badges}\n{badge_marker_end}", content)
    else:
        # Insert after existing badges (e.g. Qt 6 badge)
        badge_anchor = "[![Qt 6]"
        idx = content.find(badge_anchor)
        if idx != -1:
            line_end = content.find("\n", idx)
            content = (
                content[:line_end + 1]
                + f"{badge_marker_start}\n{cov_badges}\n{badge_marker_end}\n"
                + content[line_end + 1:]
            )
        else:
            content = f"{badge_marker_start}\n{cov_badges}\n{badge_marker_end}\n\n" + content

    # 2. Insert or replace Code Coverage Metrics section
    section_marker_start = "<!-- COVERAGE_SECTION_START -->"
    section_marker_end = "<!-- COVERAGE_SECTION_END -->"
    md_report = render_markdown_report(metrics)
    wrapped_section = f"{section_marker_start}\n{md_report}\n{section_marker_end}"

    if section_marker_start in content and section_marker_end in content:
        pattern = re.compile(rf"{re.escape(section_marker_start)}.*?{re.escape(section_marker_end)}", re.DOTALL)
        content = pattern.sub(wrapped_section, content)
    else:
        # Find a suitable location before Quickstart & Installation or at the bottom
        anchor = "## Quickstart & Installation"
        if anchor in content:
            idx = content.find(anchor)
            content = content[:idx] + f"{wrapped_section}\n\n---\n\n" + content[idx:]
        else:
            content = content + f"\n\n---\n\n{wrapped_section}\n"

    readme_path.write_text(content, encoding="utf-8")
    print(f"Updated {readme_path} with latest code coverage metrics on the GitHub main page.")


def main():
    parser = argparse.ArgumentParser(description="Generate comprehensive multi-metric code coverage reports for rmap.")
    parser.add_argument("--build-dir", type=Path, default=PROJECT_ROOT / "build", help="Path to CMake build directory")
    parser.add_argument("--source-dir", type=Path, default=PROJECT_ROOT / "src", help="Path to source directory")
    parser.add_argument("--summary", action="store_true", help="Print summary table to stdout")
    parser.add_argument("--markdown", type=Path, help="Write GitHub Markdown report to file")
    parser.add_argument("--html", type=Path, help="Write standalone HTML report to file")
    parser.add_argument("--json", type=Path, help="Write JSON report to file")
    parser.add_argument("--github-step-summary", action="store_true", help="Append Markdown report to $GITHUB_STEP_SUMMARY")
    parser.add_argument("--update-readme", nargs="?", const=str(PROJECT_ROOT / "README.md"), help="Update README.md on the GitHub main page")
    parser.add_argument("--fail-under-lines", type=float, default=0.0, help="Fail if line coverage is below this threshold")
    parser.add_argument("--fail-under-branches", type=float, default=0.0, help="Fail if branch coverage is below this threshold")
    parser.add_argument("--fail-under-functions", type=float, default=0.0, help="Fail if function coverage is below this threshold")
    parser.add_argument("--fail-under-conditions", type=float, default=0.0, help="Fail if condition coverage is below this threshold")
    parser.add_argument("--include-throw-branches", action="store_true", help="Include compiler-synthesized exception unwinding landing pads in branch metrics")
    parser.add_argument("--gcov-dir", type=Path, default=PROJECT_ROOT / "work" / "coverage" / "gcov", help="Directory where temporary gcov intermediate files are generated (default: work/coverage/gcov)")

    args = parser.parse_args()

    raw_data = parse_gcov_data(args.build_dir, args.source_dir, gcov_dir=args.gcov_dir)
    if not raw_data:
        print("No coverage data could be processed.", file=sys.stderr)
        sys.exit(1)

    metrics = compute_metrics(raw_data, exclude_throw_branches=not args.include_throw_branches)

    if args.summary or not any([args.markdown, args.html, args.json, args.github_step_summary, args.update_readme]):
        print(render_console_summary(metrics))

    if args.markdown:
        args.markdown.parent.mkdir(parents=True, exist_ok=True)
        args.markdown.write_text(render_markdown_report(metrics), encoding="utf-8")
        print(f"Wrote Markdown report to {args.markdown}")

    if args.html:
        args.html.parent.mkdir(parents=True, exist_ok=True)
        args.html.write_text(render_html_report(metrics), encoding="utf-8")
        print(f"Wrote HTML report to {args.html}")

    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(metrics, indent=2), encoding="utf-8")
        print(f"Wrote JSON report to {args.json}")

    if args.github_step_summary:
        step_summary_file = os.environ.get("GITHUB_STEP_SUMMARY")
        if step_summary_file:
            with open(step_summary_file, "a", encoding="utf-8") as f:
                f.write(render_markdown_report(metrics) + "\n")
            print(f"Appended coverage report to $GITHUB_STEP_SUMMARY ({step_summary_file})")
        else:
            print("Note: GITHUB_STEP_SUMMARY environment variable is not set.", file=sys.stderr)

    if args.update_readme:
        readme_target = Path(args.update_readme)
        update_readme(readme_target, metrics)

    # Check thresholds
    s = metrics["summary"]
    failures = []
    if s["lines"]["percent"] < args.fail_under_lines:
        failures.append(f"Line coverage {s['lines']['percent']:.2f}% is below threshold {args.fail_under_lines}%")
    if s["branches"]["percent"] < args.fail_under_branches:
        failures.append(f"Branch coverage {s['branches']['percent']:.2f}% is below threshold {args.fail_under_branches}%")
    if s["functions"]["percent"] < args.fail_under_functions:
        failures.append(f"Function coverage {s['functions']['percent']:.2f}% is below threshold {args.fail_under_functions}%")
    if s["conditions"]["percent"] < args.fail_under_conditions:
        failures.append(f"Condition coverage {s['conditions']['percent']:.2f}% is below threshold {args.fail_under_conditions}%")

    if failures:
        for f in failures:
            print(f"Error: {f}", file=sys.stderr)
        sys.exit(2)


if __name__ == "__main__":
    main()
