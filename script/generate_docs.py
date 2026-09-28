#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
Unified Documentation Generator for rmap.

Generates:
1. Interactive static HTML documentation portal (using Doxygen with optional Doxygen-Awesome-CSS).
2. Professional publication-quality User Manual PDF (rmap_user_manual.pdf).
3. Professional publication-quality Developer Reference Guide PDF (rmap_developer_guide.pdf).

Ensures all installed documentation is properly formatted (HTML and PDF)
rather than exposing raw unformatted Markdown source files.
"""

import os
import sys
import re
import shutil
import argparse
import subprocess
import tempfile
from pathlib import Path

# Documentation chapters for User Manual
USER_MANUAL_DOCS = [
    ("docs/index.md", "Overview & Architecture", "Introduction"),
    ("docs/user/getting-started.md", "Getting Started & Installation", "User Guide"),
    ("docs/user/supported-os.md", "Supported Operating Systems & Compatibility Matrix", "User Guide"),
    ("docs/user/gui-guide.md", "Interactive GUI User Guide", "User Guide"),
    ("docs/user/cli-reference.md", "Command-Line Interface & Automation", "User Guide"),
    ("docs/user/templates-and-codegen.md", "Code Generation & Templates", "User Guide"),
    ("docs/user/architecture.md", "System Architecture & Data Model", "User Guide"),
    ("templates/README.md", "Code Generation Templates Specification", "Reference & Appendices"),
    ("examples/README.md", "Reference Examples & Verification Environments", "Reference & Appendices"),
]

# Documentation chapters for Developer Guide
DEVELOPER_GUIDE_DOCS = [
    ("docs/dev/index.md", "Developer Guide Overview", "Introduction"),
    ("docs/dev/architecture.md", "C++ Subsystem Architecture", "Architecture Overview"),
    ("docs/user/architecture.md", "System Architecture & Data Model", "Architecture & Data Model"),
    ("docs/dev/standards.md", "Code & Documentation Standards", "Development Standards"),
    ("docs/dev/coverage.md", "Code Coverage & Quality Metrics", "Quality Assurance"),
    ("docs/user/templates-and-codegen.md", "Code Generation Engine & Templates", "Code Generation Architecture"),
    ("templates/README.md", "Code Generation Templates Specification", "Reference & Specifications"),
    ("examples/README.md", "Reference Examples & Verification Environments", "Reference & Specifications"),
]


BOX_MAP = {
    '┌': '+', '┐': '+', '└': '+', '┘': '+', '├': '+', '┤': '+', '┬': '+', '┴': '+', '┼': '+',
    '─': '-', '│': '|', '═': '=', '║': '|', '╒': '+', '╕': '+', '╘': '+', '╛': '+',
    '▼': 'v', '▲': '^', '►': '>', '◄': '<', '→': '->', '←': '<-', '↔': '<->',
    '✓': '[x]', '✔': '[x]', '✗': '[ ]', '✘': '[ ]', '✅': '[PASS]', 'ℹ️': '[INFO]',
    '📚': '', '🚀': '', '🛠️': '', '🛠': '', '⚡': '', '🔍': '', '🗺': '', '↳': '->',
    '—': '--', '–': '-', '…': '...', '“': '"', '”': '"', '‘': "'", '’': "'",
    '&rarr;': '->', '&harr;': '<->', '&mdash;': '--', '&amp;': '&', '&lt;': '<', '&gt;': '>',
    '•': '*', '·': '*', '°': ' deg', '×': 'x', '≤': '<=', '≥': '>=',
    '简体中文': 'Simplified Chinese',
    '日本語': 'Japanese',
    'Español': 'Spanish',
    'Deutsch': 'German',
    'Français': 'French',
    'Português': 'Portuguese',
}


def sanitize_text(text: str) -> str:
    for k, v in BOX_MAP.items():
        text = text.replace(k, v)
    return text


def escape_latex(text: str) -> str:
    text = sanitize_text(text)
    rep = [
        ('\\', r'\textbackslash{}'),
        ('&', r'\&'),
        ('%', r'\%'),
        ('$', r'\$'),
        ('#', r'\#'),
        ('_', r'\_'),
        ('{', r'\{'),
        ('}', r'\}'),
        ('~', r'\textasciitilde{}'),
        ('^', r'\textasciicircum{}'),
    ]
    for char, replacement in rep:
        text = text.replace(char, replacement)
    return text


def format_inline(text: str) -> str:
    token_re = re.compile(
        r'(?P<code>`[^`]+`|<code>[^<]+</code>)|'
        r'(?P<link>\[(?:[^\]\\]|\\.)+\]\([^)]+\))|'
        r'(?P<bold>\*\*(?:[^*]|\*[^*])+\*\*|<b>.+?</b>)|'
        r'(?P<italic>(?<!\*)\*(?:[^*]|\*\*)+\*(?!\*)|<i>.+?</i>)'
    )

    pos = 0
    out = []

    for m in token_re.finditer(text):
        start, end = m.span()
        if start > pos:
            out.append(escape_latex(text[pos:start]))
        pos = end

        if m.group('code'):
            raw = m.group('code')
            if raw.startswith('`') and raw.endswith('`'):
                c = raw[1:-1]
            else:
                c = raw[6:-7]
            out.append(r'\texttt{' + escape_latex(c) + r'}')
        elif m.group('link'):
            raw = m.group('link')
            lm = re.match(r'^\[((?:[^\]\\]|\\.)+)\]\(([^)]+)\)$', raw)
            if lm:
                ltext = format_inline(lm.group(1))
                lurl = lm.group(2)
                if lurl.startswith('@ref ') or lurl.startswith('#'):
                    out.append(r'\textbf{' + ltext + r'}')
                elif lurl.startswith('http://') or lurl.startswith('https://') or lurl.startswith('mailto:'):
                    out.append(r'\href{' + lurl + r'}{' + ltext + r'}')
                elif lurl.endswith('.html') or lurl.endswith('.md'):
                    out.append(r'\textit{' + ltext + r'}')
                else:
                    out.append(r'\href{' + lurl + r'}{' + ltext + r'}')
            else:
                out.append(escape_latex(raw))
        elif m.group('bold'):
            raw = m.group('bold')
            if raw.startswith('**') and raw.endswith('**'):
                inner = raw[2:-2]
            else:
                inner = raw[3:-4]
            out.append(r'\textbf{' + format_inline(inner) + r'}')
        elif m.group('italic'):
            raw = m.group('italic')
            if raw.startswith('*') and raw.endswith('*'):
                inner = raw[1:-1]
            else:
                inner = raw[3:-4]
            out.append(r'\textit{' + format_inline(inner) + r'}')

    if pos < len(text):
        out.append(escape_latex(text[pos:]))

    return ''.join(out)


def parse_markdown_to_latex(md_content: str, default_chapter_title: str = "", project_root: str = "", doc_path: str = "") -> str:
    lines = md_content.splitlines()
    out = []
    in_code = False
    code_lang = "text"
    code_lines = []
    in_table = False
    table_lines = []
    in_list = False
    list_type = "itemize"
    chapter_set = False

    def flush_table():
        nonlocal in_table, table_lines
        if in_table:
            if len(table_lines) >= 2:
                raw_header = [c.strip() for c in table_lines[0].split('|')[1:-1]]
                num_cols = len(raw_header)
                if num_cols > 0:
                    start_idx = 1
                    if len(table_lines) > 1 and re.match(r'^\s*\|?\s*[-:]+\s*\|', table_lines[1]):
                        start_idx = 2
                    col_spec = 'l' * (num_cols - 1) + 'X' if num_cols > 1 else 'X'
                    out.append(r'\begin{center}')
                    out.append(r'\small')
                    out.append(r'\begin{tabularx}{\textwidth}{' + col_spec + r'}')
                    out.append(r'\toprule')
                    out.append(' & '.join([r'\textbf{' + format_inline(c) + r'}' for c in raw_header]) + r' \\')
                    out.append(r'\midrule')
                    for r in table_lines[start_idx:]:
                        cells = [c.strip() for c in r.split('|')[1:-1]]
                        while len(cells) < num_cols:
                            cells.append('')
                        cells = cells[:num_cols]
                        out.append(' & '.join([format_inline(c) for c in cells]) + r' \\')
                    out.append(r'\bottomrule')
                    out.append(r'\end{tabularx}')
                    out.append(r'\end{center}')
            table_lines = []
            in_table = False

    def flush_list():
        nonlocal in_list, list_type
        if in_list:
            out.append(f'\\end{{{list_type}}}')
            in_list = False

    for line in lines:
        stripped = line.strip()
        if stripped.startswith('```'):
            flush_table()
            flush_list()
            if not in_code:
                in_code = True
                code_lang = stripped[3:].strip()
                if not code_lang:
                    code_lang = 'text'
                code_lines = []
            else:
                in_code = False
                cl_lower = code_lang.lower()
                if 'c++' in cl_lower or 'cpp' in cl_lower or cl_lower in ('c', 'h', 'hpp'):
                    lang_opt = '[language=C++]'
                elif cl_lower in ('python', 'py'):
                    lang_opt = '[language=Python]'
                elif cl_lower in ('bash', 'sh'):
                    lang_opt = '[language=bash]'
                elif cl_lower in ('verilog', 'sv', 'systemverilog'):
                    lang_opt = '[language=Verilog]'
                elif cl_lower in ('make', 'makefile'):
                    lang_opt = '[language=make]'
                else:
                    lang_opt = '[language={}]'
                out.append(f'\\begin{{lstlisting}}{lang_opt}')
                out.extend([sanitize_text(l) for l in code_lines])
                out.append('\\end{lstlisting}\n')
            continue

        if in_code:
            code_lines.append(line)
            continue

        if '|' in line and (line.strip().startswith('|') or line.strip().endswith('|')):
            flush_list()
            in_table = True
            table_lines.append(line)
            continue
        elif in_table:
            flush_table()

        if line.startswith('#'):
            flush_list()
            m = re.match(r'^(#+)\s+(.+)$', line)
            if m:
                level = len(m.group(1))
                heading_raw = m.group(2).strip()
                heading_raw = re.sub(r'\s*\{#[^}]+\}\s*$', '', heading_raw)
                htext = format_inline(heading_raw)
                if level == 1:
                    if not chapter_set:
                        out.append(f'\\chapter{{{htext}}}\n')
                        chapter_set = True
                    else:
                        out.append(f'\\section{{{htext}}}\n')
                elif level == 2:
                    out.append(f'\\section{{{htext}}}\n')
                elif level == 3:
                    out.append(f'\\subsection{{{htext}}}\n')
                else:
                    out.append(f'\\subsubsection{{{htext}}}\n')
                continue

        if stripped in ('---', '***', '___'):
            flush_list()
            out.append('\n\\bigskip\\hrule\\bigskip\n')
            continue

        m_bullet = re.match(r'^\s*[-*+]\s+(.+)$', line)
        m_num = re.match(r'^\s*(\d+)\.\s+(.+)$', line)
        if m_bullet:
            if not in_list:
                in_list = True
                list_type = 'itemize'
                out.append(f'\\begin{{{list_type}}}')
            out.append(f'  \\item {format_inline(m_bullet.group(1))}')
            continue
        elif m_num:
            if not in_list:
                in_list = True
                list_type = 'enumerate'
                out.append(f'\\begin{{{list_type}}}')
            out.append(f'  \\item {format_inline(m_num.group(2))}')
            continue
        else:
            flush_list()

        if not stripped:
            out.append('')
            continue

        # Check for image markdown: ![alt](path)
        m_img = re.match(r'^\s*!\[(.*?)\]\((.*?)\)\s*$', line)
        if m_img:
            flush_table()
            flush_list()
            alt_text = format_inline(m_img.group(1).strip())
            img_target = m_img.group(2).strip()
            img_basename = os.path.basename(img_target)
            doc_dir = os.path.dirname(os.path.abspath(doc_path)) if doc_path else (os.path.join(project_root, "docs") if project_root else "docs")
            candidate_paths = [
                os.path.normpath(os.path.join(doc_dir, img_target)),
                os.path.join(project_root, "docs", "images", img_basename) if project_root else os.path.join("docs", "images", img_basename),
                os.path.join(project_root, "docs", "images", "wavedrom", img_basename) if project_root else os.path.join("docs", "images", "wavedrom", img_basename),
            ]
            resolved_img = next((p for p in candidate_paths if os.path.isfile(p)), None)
            if resolved_img:
                clean_path = os.path.abspath(resolved_img).replace("\\", "/")
                out.append(r'\begin{figure}[htbp]')
                out.append(r'\centering')
                out.append(r'\includegraphics[width=\textwidth,height=0.40\textheight,keepaspectratio]{' + clean_path + r'}')
                if alt_text:
                    out.append(r'\caption{' + alt_text + r'}')
                out.append(r'\end{figure}' + '\n')
            continue

        # Skip or format HTML <details> and <summary> tags
        if stripped.startswith('<details>') or stripped.startswith('</details>') or stripped.startswith('<summary>') or stripped.startswith('</summary>'):
            if '<summary>' in stripped:
                summary_content = re.sub(r'</?summary>', '', stripped).strip()
                out.append(r'\paragraph{\small\textit{' + format_inline(summary_content) + r'}}' + '\n')
            continue

        out.append(format_inline(line))

    flush_table()
    flush_list()
    if not chapter_set and default_chapter_title:
        out.insert(0, f'\\chapter{{{default_chapter_title}}}\n')
    return '\n'.join(out)


def get_version(project_root: str) -> str:
    version_file = os.path.join(project_root, "VERSION")
    if os.path.exists(version_file):
        with open(version_file, "r", encoding="utf-8") as f:
            v = f.read().strip()
            if v:
                return v
    return "1.0.0"


def compile_latex_book(
    pdf_path: str,
    project_root: str,
    doc_title: str,
    doc_subtitle: str,
    doc_type_desc: str,
    doc_items: list,
    verbose: bool = False
) -> bool:
    print(f"--> Compiling {doc_title}: {pdf_path}")
    version_str = get_version(project_root)

    tex_out = []
    tex_out.append(r"""\documentclass[10pt,a4paper,oneside]{book}
\usepackage[utf8]{inputenc}
\usepackage[margin=2.2cm]{geometry}
\usepackage{hyperref}
\usepackage{booktabs}
\usepackage{tabularx}
\usepackage{listings}
\usepackage{xcolor}
\usepackage{fancyhdr}
\usepackage{titlesec}
\usepackage{parskip}
\usepackage{graphicx}

\definecolor{codebg}{rgb}{0.96,0.96,0.96}
\definecolor{codeframe}{rgb}{0.85,0.85,0.85}
\definecolor{brandblue}{rgb}{0.08,0.38,0.74}
\definecolor{accentgreen}{rgb}{0.18,0.49,0.20}

\hypersetup{
    colorlinks=true,
    linkcolor=brandblue,
    filecolor=brandblue,
    urlcolor=brandblue,
    citecolor=accentgreen,
    pdftitle={""" + doc_title + r"""},
    pdfauthor={Ezequiel Alves <alvesel@gmail.com>},
}

\lstset{
    backgroundcolor=\color{codebg},
    basicstyle=\ttfamily\footnotesize,
    breaklines=true,
    frame=single,
    rulecolor=\color{codeframe},
    tabsize=2,
    showstringspaces=false
}

\pagestyle{fancy}
\fancyhf{}
\fancyhead[L]{\textsl{\leftmark}}
\fancyhead[R]{\thepage}
\fancyfoot[C]{\footnotesize rmap --- """ + doc_title + r"""}
\renewcommand{\headrulewidth}{0.4pt}
\renewcommand{\footrulewidth}{0.4pt}

\begin{document}

\begin{titlepage}
\centering
\vspace*{3cm}
{\Huge\textbf{\color{brandblue}rmap}}\\[1.5cm]
{\Large\textbf{""" + doc_subtitle + r"""}}\\[0.8cm]
{\large """ + doc_type_desc + r"""}\\[2.5cm]
\textbf{Version """ + version_str + r"""}\\[0.4cm]
\textbf{Author: Ezequiel Alves}\\[0.4cm]
\textbf{GitHub: \url{https://github.com/zkalves/rmap}}\\[0.6cm]
\textsl{Mozilla Public License 2.0 (MPL-2.0) with Output Exception}\\[1cm]
\today
\vfill
\end{titlepage}

\frontmatter
\tableofcontents

\mainmatter
""")

    current_part = ""
    for rel_path, title, part in doc_items:
        full_path = os.path.join(project_root, rel_path)
        if part != current_part:
            current_part = part
            tex_out.append(f"\n\\part{{{part}}}\n")
        if os.path.exists(full_path):
            with open(full_path, "r", encoding="utf-8") as f:
                content = f.read()
            tex_out.append(parse_markdown_to_latex(content, default_chapter_title=title, project_root=project_root, doc_path=full_path))
        else:
            if verbose:
                print(f"Warning: Chapter file not found: {rel_path}")

    tex_out.append(r"\end{document}")

    with tempfile.TemporaryDirectory(prefix="rmap_doc_") as tmpdir:
        tex_file = os.path.join(tmpdir, "document.tex")
        with open(tex_file, "w", encoding="utf-8") as f:
            f.write("\n".join(tex_out))

        if shutil.which("pdflatex"):
            for pass_num in range(1, 3):
                res = subprocess.run(
                    ["pdflatex", "-interaction=nonstopmode", f"-output-directory={tmpdir}", tex_file],
                    capture_output=True,
                    text=True
                )
                if verbose:
                    print(f"pdflatex pass {pass_num} return code: {res.returncode}")

            gen_pdf = os.path.join(tmpdir, "document.pdf")
            if os.path.exists(gen_pdf):
                os.makedirs(os.path.dirname(os.path.abspath(pdf_path)), exist_ok=True)
                shutil.copy2(gen_pdf, pdf_path)
                pdf_size_kb = os.path.getsize(pdf_path) / 1024
                print(f"--> Successfully created: {pdf_path} ({pdf_size_kb:.1f} KB)")
                return True
            else:
                print(f"Error: pdflatex did not produce document.pdf for {doc_title}")
                if verbose:
                    log_file = os.path.join(tmpdir, "document.log")
                    if os.path.exists(log_file):
                        with open(log_file, "r") as lf:
                            print(lf.read()[-2000:])
                return False
        else:
            print("Warning: pdflatex is not installed on this system.")
            return False


def generate_user_manual(pdf_path: str, project_root: str, verbose: bool = False) -> bool:
    return compile_latex_book(
        pdf_path=pdf_path,
        project_root=project_root,
        doc_title="rmap User Manual",
        doc_subtitle="Hardware Register Map Designer & Model Generator",
        doc_type_desc="Comprehensive User Guide & Reference Manual",
        doc_items=USER_MANUAL_DOCS,
        verbose=verbose
    )


def generate_developer_guide(pdf_path: str, project_root: str, verbose: bool = False) -> bool:
    return compile_latex_book(
        pdf_path=pdf_path,
        project_root=project_root,
        doc_title="rmap Developer Guide",
        doc_subtitle="Hardware Register Map Designer & Model Generator",
        doc_type_desc="C++ Architecture & Internal API Reference Manual",
        doc_items=DEVELOPER_GUIDE_DOCS,
        verbose=verbose
    )


def ensure_screenshots(project_root: str, verbose: bool = False) -> bool:
    """
    Ensures GUI screenshots exist in docs/images.
    If the capture_screenshots binary is built, runs it to capture fresh offscreen images.
    """
    img_dir = os.path.join(project_root, "docs", "images")
    os.makedirs(img_dir, exist_ok=True)

    candidates = [
        os.path.join(project_root, "build", "bin", "capture_screenshots"),
        os.path.join(project_root, "bin", "capture_screenshots"),
    ]
    capture_bin = next((c for c in candidates if os.path.isfile(c) and os.access(c, os.X_OK)), None)
    if capture_bin:
        print(f"--> Capturing fresh documentation GUI screenshots using: {os.path.relpath(capture_bin, project_root)}")
        env = os.environ.copy()
        env["QT_QPA_PLATFORM"] = "offscreen"
        res = subprocess.run([capture_bin, img_dir], env=env, cwd=project_root, capture_output=not verbose, text=True)
        if res.returncode == 0:
            print("✓ Successfully generated GUI screenshots into docs/images")
            return True
        elif verbose and res.stderr:
            print(f"[WARNING] capture_screenshots returned error: {res.stderr}", file=sys.stderr)
    return True


def ensure_wavedrom_diagrams(project_root: str, verbose: bool = False) -> bool:
    """
    Ensures WaveDrom timing diagrams exist in docs/images/wavedrom.
    Renders SVG and PNG waveforms using script/wavedrom_renderer.py.
    """
    wd_dir = os.path.join(project_root, "docs", "images", "wavedrom")
    os.makedirs(wd_dir, exist_ok=True)
    renderer_script = os.path.join(project_root, "script", "wavedrom_renderer.py")
    if os.path.isfile(renderer_script):
        if verbose:
            print(f"--> Generating WaveDrom timing diagrams into: {os.path.relpath(wd_dir, project_root)}")
        try:
            sys.path.insert(0, os.path.join(project_root, "script"))
            from wavedrom_renderer import render_all_standard_wavedroms
            render_all_standard_wavedroms(wd_dir)
            if verbose:
                print("✓ Successfully generated WaveDrom timing diagrams into docs/images/wavedrom")
            return True
        except Exception as e:
            if verbose:
                print(f"[WARNING] WaveDrom renderer error: {e}", file=sys.stderr)
            return False
    return True


def run_doxygen(project_root: str, html_dir: str = "", verbose: bool = False, required: bool = False) -> bool:
    doxygen_bin = shutil.which("doxygen")
    if not doxygen_bin:
        msg = (
            "\n======================================================================\n"
            "ERROR: Doxygen is required to generate the HTML documentation portal,\n"
            "       but 'doxygen' was not found on PATH.\n"
            "======================================================================\n"
            "Please install Doxygen:\n"
            "  - Ubuntu/Debian: sudo apt-get install -y doxygen graphviz\n"
            "  - macOS:         brew install doxygen graphviz\n"
            "  - Fedora/RHEL:   sudo dnf install doxygen graphviz\n"
            "  - Arch Linux:    sudo pacman -S doxygen graphviz\n"
        )
        if required:
            print(msg, file=sys.stderr)
            return False
        else:
            print("[INFO] Doxygen not found on PATH. Skipping Doxygen HTML generation.\n"
                  "       Install doxygen (e.g. 'sudo apt-get install -y doxygen graphviz') to generate HTML documentation.")
            return True

    doxyfile = os.path.join(project_root, "docs", "Doxyfile")
    if not os.path.exists(doxyfile):
        print(f"[ERROR] Doxyfile not found at: {doxyfile}", file=sys.stderr)
        return False

    with open(doxyfile, "r", encoding="utf-8") as f:
        doxy_cfg = f.read()

    # Override OUTPUT_DIRECTORY if custom html_dir is supplied and different from default
    target_out = html_dir if html_dir else os.path.join(project_root, "_site")
    if html_dir and os.path.abspath(html_dir) != os.path.abspath(os.path.join(project_root, "_site")):
        doxy_cfg += f"\nOUTPUT_DIRECTORY = {html_dir}\n"

    # Solarized CSS theme: check if available
    theme_candidates = [
        os.path.join(project_root, "docs", "theme", "solarized.css"),
        os.path.join(project_root, "docs", "theme", "doxygen-awesome.css"),
        os.path.join(project_root, "docs", "doxygen-awesome-css", "doxygen-awesome.css"),
        os.path.join(project_root, "docs", "doxygen-awesome.css"),
    ]
    theme_path = next((p for p in theme_candidates if os.path.isfile(p)), None)
    if theme_path:
        rel_theme = os.path.relpath(theme_path, project_root)
        print(f"--> Using Doxygen Solarized CSS theme: {rel_theme}")
        doxy_cfg += f"\nHTML_EXTRA_STYLESHEET = {rel_theme}\n"
    else:
        print("[INFO] Doxygen Solarized CSS not found. Using standard Doxygen styling.")
        doxy_cfg += "\nHTML_EXTRA_STYLESHEET =\n"

    print(f"--> Generating Doxygen HTML Documentation Portal into: {target_out}")
    res = subprocess.run([doxygen_bin, "-"], input=doxy_cfg, cwd=project_root, capture_output=not verbose, text=True)
    if res.returncode != 0:
        print(f"[ERROR] Doxygen failed with return code {res.returncode}", file=sys.stderr)
        if res.stderr:
            print(res.stderr, file=sys.stderr)
        return False
    elif not verbose and res.stderr and "warning:" in res.stderr.lower():
        print(f"[WARNING] Doxygen reported warnings during documentation generation:\n{res.stderr}", file=sys.stderr)

    print(f"✓ Successfully rendered HTML documentation with Doxygen into: {target_out}")
    postprocess_html(target_out, project_root=project_root)
    return True


def postprocess_html(html_dir: str, project_root: str = ""):
    """
    Post-process generated HTML documentation:
    1. Rewrites relative Markdown or file reference stub links (*_8md.html) to their canonical documentation pages.
    2. Overwrites empty Doxygen file stub pages (*_8md.html) with instant meta-refresh redirects to their canonical pages.
    3. Provides a stub dashboard if coverage/index.html is not yet generated.
    """
    if not os.path.isdir(html_dir):
        return

    redirect_map = {
        "getting-started_8md.html": "getting_started.html",
        "supported-os_8md.html": "supported_os.html",
        "gui-guide_8md.html": "gui_guide.html",
        "cli-reference_8md.html": "cli_reference.html",
        "templates-and-codegen_8md.html": "templates_codegen.html",
        "user_2architecture_8md.html": "architecture.html",
        "architecture_8md.html": "architecture.html",
        "dev_2architecture_8md.html": "dev_architecture.html",
        "dev_2standards_8md.html": "dev_standards.html",
        "standards_8md.html": "dev_standards.html",
        "dev_2coverage_8md.html": "dev_coverage.html",
        "coverage_8md.html": "dev_coverage.html",
        "dev_2index_8md.html": "dev_guide.html",
        "user_2index_8md.html": "user_guide.html",
        "index_8md.html": "index.html",
    }

    file_map = {
        "getting-started.md": "getting_started.html",
        "supported-os.md": "supported_os.html",
        "gui-guide.md": "gui_guide.html",
        "cli-reference.md": "cli_reference.html",
        "templates-and-codegen.md": "templates_codegen.html",
        "architecture.md": "architecture.html",
        "user/architecture.md": "architecture.html",
        "dev/architecture.md": "dev_architecture.html",
        "dev/standards.md": "dev_standards.html",
        "dev/coverage.md": "dev_coverage.html",
        "dev/index.md": "dev_guide.html",
        "user/index.md": "user_guide.html",
        "index.md": "index.html",
    }

    full_rewrite = {}
    full_rewrite.update(redirect_map)
    full_rewrite.update(file_map)

    # 1. Scan and rewrite links in all HTML files
    for root, _, files in os.walk(html_dir):
        for f in files:
            if not f.endswith(".html"):
                continue
            fpath = os.path.join(root, f)
            with open(fpath, "r", encoding="utf-8", errors="ignore") as fp:
                content = fp.read()

            modified = False
            for src_ref, tgt_page in full_rewrite.items():
                pattern = f'href="{src_ref}"'
                if pattern in content:
                    content = content.replace(pattern, f'href="{tgt_page}"')
                    modified = True
                pattern_q = f"href='{src_ref}'"
                if pattern_q in content:
                    content = content.replace(pattern_q, f"href='{tgt_page}'")
                    modified = True

            if modified:
                with open(fpath, "w", encoding="utf-8") as fp:
                    fp.write(content)

    # 2. Overwrite stub files with meta-refresh redirects
    for stub_file, canonical_page in redirect_map.items():
        stub_path = os.path.join(html_dir, stub_file)
        redirect_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta http-equiv="refresh" content="0; url={canonical_page}">
<link rel="canonical" href="{canonical_page}">
<title>Redirecting to {canonical_page}...</title>
<script>window.location.replace("{canonical_page}");</script>
</head>
<body>
<p>This page has moved. Redirecting to <a href="{canonical_page}">{canonical_page}</a>...</p>
</body>
</html>
"""
        with open(stub_path, "w", encoding="utf-8") as fp:
            fp.write(redirect_content)

    # 3. Create placeholder for coverage/index.html if missing
    cov_dir = os.path.join(html_dir, "coverage")
    cov_index = os.path.join(cov_dir, "index.html")
    if not os.path.exists(cov_index):
        os.makedirs(cov_dir, exist_ok=True)
        placeholder = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>rmap Code Coverage Report</title>
<style>
body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #002b36; color: #839496; padding: 40px; text-align: center; }
.card { background: #073642; border-radius: 8px; max-width: 600px; margin: 40px auto; padding: 30px; box-shadow: 0 4px 12px rgba(0,0,0,0.3); }
h1 { color: #268bd2; margin-top: 0; }
code { background: #002b36; color: #2aa198; padding: 3px 8px; border-radius: 4px; font-size: 0.95em; }
a { color: #2aa198; text-decoration: none; }
a:hover { text-decoration: underline; }
</style>
</head>
<body>
<div class="card">
<h1>rmap Code Coverage Dashboard</h1>
<p>Live interactive coverage reports are generated during CI pipeline execution or locally via:</p>
<p><code>make coverage-report</code></p>
<p><a href="../index.html">&larr; Return to Documentation Portal</a></p>
</div>
</body>
</html>
"""
        with open(cov_index, "w", encoding="utf-8") as fp:
            fp.write(placeholder)

    # 4. Create .nojekyll to ensure GitHub Pages serves static files directly
    nojekyll_path = os.path.join(html_dir, ".nojekyll")
    if not os.path.exists(nojekyll_path):
        with open(nojekyll_path, "w", encoding="utf-8") as fp:
            pass

    # 5. Sanitize any spurious </blockquote> heading artifacts produced by Doxygen
    for root, _, files in os.walk(html_dir):
        for f in files:
            fpath = os.path.join(root, f)
            if f.endswith(".html"):
                with open(fpath, "r", encoding="utf-8", errors="ignore") as fp:
                    content = fp.read()
                cleaned = re.sub(r'<h[1-6]><a class="anchor"[^>]*></a>\s*&lt;/blockquote&gt;\s*</h[1-6]>\n?', '', content)
                if cleaned != content:
                    with open(fpath, "w", encoding="utf-8") as fp:
                        fp.write(cleaned)
            elif f.endswith(".js"):
                with open(fpath, "r", encoding="utf-8", errors="ignore") as fp:
                    content = fp.read()
                cleaned = re.sub(r'\s*\[\s*"</blockquote>"[^]]*\],?\n?', '', content)
                cleaned = re.sub(r',\s*(\])', r'\1', cleaned)
                if cleaned != content:
                    with open(fpath, "w", encoding="utf-8") as fp:
                        fp.write(cleaned)

    # 6. Synchronize documentation images into output HTML directory
    if project_root:
        docs_img_dir = os.path.join(project_root, "docs", "images")
        if os.path.isdir(docs_img_dir):
            out_img_dir = os.path.join(html_dir, "images")
            os.makedirs(out_img_dir, exist_ok=True)
            for root, _, files in os.walk(docs_img_dir):
                rel_dir = os.path.relpath(root, docs_img_dir)
                target_sub = os.path.join(out_img_dir, rel_dir) if rel_dir != "." else out_img_dir
                os.makedirs(target_sub, exist_ok=True)
                for f in files:
                    if f.lower().endswith((".png", ".jpg", ".jpeg", ".svg")):
                        src_f = os.path.join(root, f)
                        shutil.copy2(src_f, os.path.join(target_sub, f))
                        shutil.copy2(src_f, os.path.join(html_dir, f))

    # 7. Rewrite relative image paths (../images/ -> images/) for direct HTML viewing
    for root, _, files in os.walk(html_dir):
        for f in files:
            if not f.endswith(".html"):
                continue
            fpath = os.path.join(root, f)
            with open(fpath, "r", encoding="utf-8", errors="ignore") as fp:
                c = fp.read()
            if 'src="../images/' in c:
                c = c.replace('src="../images/', 'src="images/')
                with open(fpath, "w", encoding="utf-8") as fp:
                    fp.write(c)

    # 8. Sanitize unparsed Markdown bold artifacts (**...**) that Doxygen left unrendered
    def sanitize_bold_artifacts(html_text: str) -> str:
        blocks = []
        def save_block(m):
            blocks.append(m.group(0))
            return f"<!--__PROTECTED_DOC_BLOCK_{len(blocks)-1}__-->"

        prot_pattern = re.compile(
            r'(<pre\b[^>]*>[\s\S]*?</pre>|<div class="fragment"[\s\S]*?</div>\s*</div>|<script\b[^>]*>[\s\S]*?</script>|<style\b[^>]*>[\s\S]*?</style>)',
            re.IGNORECASE
        )
        protected = prot_pattern.sub(save_block, html_text)
        sanitized = re.sub(r'\*\*(.+?)\*\*', r'<b>\1</b>', protected)
        for idx, block in enumerate(blocks):
            sanitized = sanitized.replace(f"<!--__PROTECTED_DOC_BLOCK_{idx}__-->", block)
        return sanitized

    for root, _, files in os.walk(html_dir):
        for f in files:
            if not f.endswith(".html"):
                continue
            fpath = os.path.join(root, f)
            with open(fpath, "r", encoding="utf-8", errors="ignore") as fp:
                c = fp.read()
            cleaned = sanitize_bold_artifacts(c)
            if cleaned != c:
                with open(fpath, "w", encoding="utf-8") as fp:
                    fp.write(cleaned)


def generate_html(html_dir: str, project_root: str, verbose: bool = False) -> bool:
    os.makedirs(html_dir, exist_ok=True)
    return run_doxygen(project_root, html_dir=html_dir, verbose=verbose, required=False)



def main():
    parser = argparse.ArgumentParser(
        description="Generate HTML, User Manual PDF, and Developer Guide PDF for rmap."
    )
    parser.add_argument(
        "--project-root",
        default=str(Path(__file__).resolve().parent.parent),
        help="Path to rmap repository root"
    )
    parser.add_argument(
        "--build-dir",
        default="",
        help="CMake build directory"
    )
    parser.add_argument(
        "--html-dir",
        default="",
        help="Target output directory for HTML documentation"
    )
    parser.add_argument(
        "--pdf-dir",
        default="",
        help="Target directory for output PDF manuals"
    )
    parser.add_argument(
        "--user-pdf",
        default="",
        help="Target output path for User Manual PDF (default: rmap_user_manual.pdf)"
    )
    parser.add_argument(
        "--dev-pdf",
        default="",
        help="Target output path for Developer Guide PDF (default: rmap_developer_guide.pdf)"
    )
    parser.add_argument(
        "--html-only",
        action="store_true",
        help="Generate only the HTML documentation"
    )
    parser.add_argument(
        "--pdf-only",
        action="store_true",
        help="Generate both User Manual and Developer Guide PDFs"
    )
    parser.add_argument(
        "--user-only",
        action="store_true",
        help="Generate only the User Manual PDF"
    )
    parser.add_argument(
        "--dev-only",
        action="store_true",
        help="Generate only the Developer Guide PDF"
    )
    parser.add_argument(
        "--doxygen",
        action="store_true",
        help="Generate C++ API documentation using Doxygen (strictly requires Doxygen on PATH)"
    )
    parser.add_argument(
        "--screenshots",
        action="store_true",
        help="Capture fresh GUI screenshots using capture_screenshots binary before generating documentation"
    )
    parser.add_argument(
        "--wavedrom",
        action="store_true",
        help="Generate WaveDrom timing diagrams into docs/images/wavedrom"
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Clean target output directories before generation"
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose output"
    )

    args = parser.parse_args()
    project_root = os.path.abspath(args.project_root)

    # Ensure documentation GUI screenshots are present and refreshed if binary is available
    ensure_screenshots(project_root, verbose=args.verbose)
    # Ensure WaveDrom timing diagrams are generated
    ensure_wavedrom_diagrams(project_root, verbose=args.verbose)

    if args.wavedrom:
        sys.exit(0)

    if args.doxygen:
        ok = run_doxygen(project_root, verbose=args.verbose, required=True)
        sys.exit(0 if ok else 1)

    # Determine default HTML directory
    if args.html_dir:
        html_dir = os.path.abspath(args.html_dir)
    elif args.build_dir:
        html_dir = os.path.abspath(os.path.join(args.build_dir, "docs", "html"))
    else:
        html_dir = os.path.abspath(os.path.join(project_root, "_site"))

    # Determine default PDF directory
    if args.pdf_dir:
        pdf_dir = os.path.abspath(args.pdf_dir)
    elif args.build_dir:
        pdf_dir = os.path.abspath(os.path.join(args.build_dir, "docs"))
    else:
        pdf_dir = os.path.abspath(os.path.join(html_dir, "pdf"))

    # Determine individual PDF paths
    user_pdf = os.path.abspath(args.user_pdf) if args.user_pdf else os.path.join(pdf_dir, "rmap_user_manual.pdf")
    dev_pdf = os.path.abspath(args.dev_pdf) if args.dev_pdf else os.path.join(pdf_dir, "rmap_developer_guide.pdf")

    # Determine what to build
    build_user_pdf = False
    build_dev_pdf = False
    build_html = False

    if args.user_only:
        build_user_pdf = True
    elif args.dev_only:
        build_dev_pdf = True
    elif args.pdf_only:
        build_user_pdf = True
        build_dev_pdf = True
    elif args.html_only:
        build_html = True
    else:
        # Default: build everything
        build_html = True
        build_user_pdf = True
        build_dev_pdf = True

    if args.clean:
        if build_html and os.path.exists(html_dir):
            shutil.rmtree(html_dir)
        if build_user_pdf and os.path.exists(user_pdf):
            os.remove(user_pdf)
        if build_dev_pdf and os.path.exists(dev_pdf):
            os.remove(dev_pdf)

    print("==========================================")
    print("rmap Comprehensive Documentation Generator")
    print("==========================================")
    print(f"Project Root:        {project_root}")
    if build_html:
        print(f"HTML Output:         {html_dir}")
    if build_user_pdf:
        print(f"User Manual PDF:     {user_pdf}")
    if build_dev_pdf:
        print(f"Developer Guide PDF: {dev_pdf}")

    success = True

    if build_html:
        ok_html = generate_html(html_dir, project_root, verbose=args.verbose)
        if not ok_html:
            success = False
    elif args.dev_only:
        ok_dox = run_doxygen(project_root, verbose=args.verbose, required=True)
        if not ok_dox:
            success = False

    if build_user_pdf:
        ok_user = generate_user_manual(user_pdf, project_root, verbose=args.verbose)
        if not ok_user:
            success = False
        # Sync into HTML website assets if generating the _site web portal
        if ok_user and os.path.exists(user_pdf) and os.path.abspath(html_dir) == os.path.abspath(os.path.join(project_root, "_site")):
            site_pdf_dir = os.path.join(html_dir, "pdf")
            os.makedirs(site_pdf_dir, exist_ok=True)
            site_dest = os.path.join(site_pdf_dir, "rmap_user_manual.pdf")
            if os.path.abspath(site_dest) != os.path.abspath(user_pdf):
                shutil.copy2(user_pdf, site_dest)

    if build_dev_pdf:
        ok_dev = generate_developer_guide(dev_pdf, project_root, verbose=args.verbose)
        if not ok_dev:
            success = False
        # Sync into HTML website assets if generating the _site web portal
        if ok_dev and os.path.exists(dev_pdf) and os.path.abspath(html_dir) == os.path.abspath(os.path.join(project_root, "_site")):
            site_pdf_dir = os.path.join(html_dir, "pdf")
            os.makedirs(site_pdf_dir, exist_ok=True)
            site_dest = os.path.join(site_pdf_dir, "rmap_developer_guide.pdf")
            if os.path.abspath(site_dest) != os.path.abspath(dev_pdf):
                shutil.copy2(dev_pdf, site_dest)

    print("==========================================")
    print("Documentation Generation Completed!")
    print("==========================================")
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()

