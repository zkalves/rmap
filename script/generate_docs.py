#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
Unified Documentation Generator for rmap.

Generates:
1. Interactive static HTML documentation portal (using Pelican or built-in HTML renderer).
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
    ("docs/user/gui-guide.md", "Interactive GUI User Guide", "User Guide"),
    ("docs/user/cli-reference.md", "Command-Line Interface & Automation", "User Guide"),
    ("docs/user/templates-and-codegen.md", "Code Generation & Templates", "User Guide"),
    ("docs/user/architecture.md", "System Architecture & Data Model", "User Guide"),
    ("templates/README.md", "Code Generation Templates Specification", "Reference & Appendices"),
    ("examples/README.md", "Reference Examples & Verification Environments", "Reference & Appendices"),
]

# Documentation chapters for Developer Guide
DEVELOPER_GUIDE_DOCS = [
    ("docs/dev/index.md", "Developer Architecture Index", "Architecture Overview"),
    ("docs/dev/rmap.md", "Application Entry Point", "Application & UI Controllers"),
    ("docs/dev/RegMapWindow.md", "Main Window Controller", "Application & UI Controllers"),
    ("docs/dev/RegConfigWindow.md", "Project Configuration Dialog", "Application & UI Controllers"),
    ("docs/dev/PreferencesWindow.md", "Preferences Dialog", "Application & UI Controllers"),
    ("docs/dev/AboutWindow.md", "About Dialog", "Application & UI Controllers"),
    ("docs/dev/RegMapTreeModel.md", "Register Map Tree Model", "Data Model & Hierarchy"),
    ("docs/dev/RegMapTreeItem.md", "Hierarchical Tree Items", "Data Model & Hierarchy"),
    ("docs/dev/RegMapTreeView.md", "Custom Tree View", "Data Model & Hierarchy"),
    ("docs/dev/RegMapDelegate.md", "Item Delegates & Fast Cell Editors", "Data Model & Hierarchy"),
    ("docs/dev/UndoCommands.md", "Undo & Redo Command Architecture", "Data Model & Hierarchy"),
    ("docs/dev/RegBitfieldBarWidget.md", "Bitfield Slice Bar Widget", "Interactive Visualizers"),
    ("docs/dev/BlockMemoryMapWidget.md", "Memory Map Diagram Widget", "Interactive Visualizers"),
    ("docs/dev/FormatManager.md", "Multi-Format Registry", "Serialization & Formats"),
    ("docs/dev/SerializationContext.md", "Serialization Framework", "Serialization & Formats"),
    ("docs/dev/CodeGenerator.md", "Inja Template Code Generator", "Utilities & Services"),
    ("docs/dev/ThemeManager.md", "Theme & Accessibility Engine", "Utilities & Services"),
    ("docs/dev/LanguageManager.md", "Internationalization & Localization", "Utilities & Services"),
    ("docs/dev/PathUtils.md", "Path Resolution & Environment Expansion", "Utilities & Services"),
    ("docs/dev/AppSettings.md", "Settings & Geometry Persistence", "Utilities & Services"),
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
    parts = []
    tokens = re.split(r'(`[^`]+`)', text)
    for token in tokens:
        if token.startswith('`') and token.endswith('`') and len(token) >= 2:
            safe_code = escape_latex(token[1:-1])
            parts.append(r'\texttt{' + safe_code + r'}')
        else:
            sub_tokens = re.split(r'(\*\*[^*]+\*\*|\*[^*]+\*|\[[^\]]+\]\([^)]+\))', token)
            for st in sub_tokens:
                if st.startswith('**') and st.endswith('**') and len(st) >= 4:
                    parts.append(r'\textbf{' + escape_latex(st[2:-2]) + r'}')
                elif st.startswith('*') and st.endswith('*') and len(st) >= 2:
                    parts.append(r'\textit{' + escape_latex(st[1:-1]) + r'}')
                elif st.startswith('[') and '](' in st and st.endswith(')'):
                    m = re.match(r'^\[([^\]]+)\]\(([^)]+)\)$', st)
                    if m:
                        link_text = escape_latex(m.group(1))
                        link_url = m.group(2)
                        parts.append(r'\href{' + link_url + r'}{' + link_text + r'}')
                    else:
                        parts.append(escape_latex(st))
                else:
                    parts.append(escape_latex(st))
    return ''.join(parts)


def parse_markdown_to_latex(md_content: str, default_chapter_title: str = "") -> str:
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
                htext = format_inline(m.group(2).strip())
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
    pdfauthor={rmap Project Contributors},
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
{\large """ + doc_type_desc + r"""}\\[3cm]
\textbf{Version """ + version_str + r"""}\\[0.5cm]
\textsl{Mozilla Public License 2.0 (MPL-2.0)}\\[1cm]
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
            tex_out.append(parse_markdown_to_latex(content, default_chapter_title=title))
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


def generate_html(html_dir: str, project_root: str, verbose: bool = False) -> bool:
    print(f"--> Generating HTML Documentation Portal: {html_dir}")
    os.makedirs(html_dir, exist_ok=True)
    docs_source_dir = os.path.join(project_root, "docs")
    site_cache_dir = os.path.join(project_root, "_site")

    env = os.environ.copy()
    py_pkg = os.path.join(project_root, ".python_packages")
    if os.path.exists(py_pkg):
        env["PYTHONPATH"] = f"{py_pkg}:{env.get('PYTHONPATH', '')}"

    pelican_cmd = None
    if subprocess.run([sys.executable, "-m", "pelican", "--version"], env=env, capture_output=True).returncode == 0:
        pelican_cmd = [sys.executable, "-m", "pelican", ".", "-s", "pelicanconf.py", "-o", os.path.abspath(html_dir)]
    elif shutil.which("pelican"):
        pelican_cmd = ["pelican", ".", "-s", "pelicanconf.py", "-o", os.path.abspath(html_dir)]

    if pelican_cmd:
        res = subprocess.run(pelican_cmd, cwd=docs_source_dir, env=env, capture_output=not verbose)
        if res.returncode == 0:
            print(f"--> Successfully rendered HTML portal with Pelican into: {html_dir}")
            return True
        else:
            print(f"Warning: Pelican rendering failed with return code {res.returncode}. Falling back to cached assets.")

    if os.path.exists(site_cache_dir) and os.path.abspath(site_cache_dir) != os.path.abspath(html_dir):
        shutil.copytree(site_cache_dir, html_dir, dirs_exist_ok=True)
        print(f"--> Successfully deployed pre-rendered HTML portal from _site into: {html_dir}")
        return True

    return True


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
