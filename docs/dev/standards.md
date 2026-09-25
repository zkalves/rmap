# Code & Documentation Standards {#dev_standards}

To maintain code quality, maintainability, and architectural integrity across **rmap**, all contributions must strictly adhere to the following development and documentation standards.

---

## C++ Code Documentation Standards

All class interfaces, method signatures, signals, slots, inheritance hierarchies, and data structures are documented directly within the C++ source headers (`src/` and `src/format/`) using standard Doxygen docstrings.

Developers must follow these formatting standards in all C++ headers:

```cpp
/**
 * @class ExampleManager
 * @brief Thread-safe singleton managing subsystem lifecycle and configuration.
 *
 * Details on the internal architecture, thread safety invariants, and Qt
 * model-view integration contracts.
 */
class ExampleManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Retrieve the global singleton instance.
     * @return Reference to the singleton instance.
     */
    static ExampleManager &instance();

    /**
     * @brief Register a new format handler.
     * @param format Extension or format identifier (e.g. "svd").
     * @param handler Owning pointer to the format handler.
     * @return True if registration succeeded without conflict.
     */
    bool registerHandler(const QString &format, std::unique_ptr<FormatHandler> handler);

signals:
    /**
     * @brief Emitted whenever active configuration parameters change.
     * @param key Modified configuration key.
     */
    void configChanged(const QString &key);
};
```

---

## Documentation & Implementation Lockstep Invariant

To ensure canonical documentation and C++ implementation never diverge, **rmap** enforces strict bidirectional synchronization across all pull requests, commits, and autonomous agent tasks:

1. **Documentation &rarr; Implementation**:
   - Any modification or addition to `docs/` **must** be accompanied by matching implementation updates in `src/`, `templates/`, and `tests/`.
   - Documentation must never run ahead of working, tested code. Pull requests modifying specifications without updating code are rejected unless explicitly tagged `[doc-only]` for non-functional typo or grammar corrections.

2. **Implementation &rarr; Documentation**:
   - Any modification in `src/` or `templates/` that alters CLI options, register access semantics, public APIs, format serializers, or template outputs **must** be flagged for documentation review.
   - If code is updated without modifying `docs/` directly, the commit or pull request must include an explicit `DOC-FLAG: <reason/tracking issue>` tag.

3. **Automated Verification**:
   - **Static Parity Checks**: `python3 script/check_doc_sync.py --static` verifies 100% bidirectional parity for all CLI options (`src/main.cpp` &harr; [docs/user/cli-reference.md](@ref cli_reference)), format handlers (`src/format/` &harr; [docs/user/architecture.md](@ref architecture)), and template deliverables (`templates/` &harr; [docs/user/templates-and-codegen.md](@ref templates_codegen)).
   - **CTest Integration**: Verified automatically as part of `ctest` via `test_ArchitecturalInvariants` and `test_DocImplementationSync`.
   - **Git Hooks & CI**: Enforced by `.git/hooks/pre-commit`, `.git/hooks/commit-msg`, and the GitHub Actions CI pipeline.

---

[Back to Developer Guide](@ref dev_guide)
