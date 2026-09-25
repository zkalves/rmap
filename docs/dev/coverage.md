# Code Coverage & Quality Metrics {#dev_coverage}

**rmap** uses compiler-based `gcov` instrumentation and an automated multi-metric coverage analysis engine (`script/generate_coverage.py`) to verify test quality across the C++ codebase.

---

## Coverage Metrics Evaluated

The engine analyzes six compiler-level coverage metrics:
- **Line Coverage**: Ratio of executed executable statements.
- **Function Coverage**: Ratio of executed functions and methods.
- **Branch Coverage**: Ratio of executed conditional branch paths (compiler-generated exception unwinding landing pads are excluded by default).
- **Condition Coverage**: MC/DC condition coverage on boolean expressions.
- **Call Coverage**: Ratio of executed function call sites.
- **Block Coverage**: Basic block execution ratio from compiler control flow graphs (CFG).

---

## Running Coverage Locally

To build with coverage instrumentation, run the test suites, and generate local reports:

```bash
# Run unit and template tests with coverage instrumentation and output a console summary
make coverage

# Generate full HTML, Markdown, and JSON reports
make coverage-report
```

Output reports are generated in `work/coverage/`:
- `work/coverage/index.html`: Interactive, searchable HTML dashboard with line-by-line profiling and call graphs.
- `work/coverage/coverage.md`: Formatted Markdown summary.
- `work/coverage/coverage.json`: Machine-readable JSON metrics for CI dashboards.

---

## Direct Script Execution & Quality Gates

The coverage script can also be executed directly with custom pass/fail threshold gates:

```bash
python3 script/generate_coverage.py \
    --build-dir build \
    --summary \
    --html work/coverage/index.html \
    --fail-under-lines 90.0 \
    --fail-under-branches 80.0
```

Continuous coverage reports and interactive call graphs are published live on the [rmap Coverage Dashboard](coverage/index.html).

---

[Back to Developer Guide](@ref dev_guide)
