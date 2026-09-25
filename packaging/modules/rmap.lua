-- -*- lua -*-
-- ============================================================================
-- rmap Environment Modulefile (Lmod)
-- Hardware Register Map Designer & Model Generator
-- ============================================================================

whatis([[Name: rmap]])
whatis([[Version: 0.2.0]])
whatis([[Category: EDA / Hardware Design & Verification]])
whatis([[Description: Modern C++17 / Qt 6 Hardware Register Map Designer & RAL Model Generator]])
whatis([[URL: https://github.com/zkalves/rmap]])

help([[
The rmap module configures your shell environment for the rmap hardware register
map designer, linter, semantic diff engine, and code generation toolchain.

Available Commands:
  rmap                     Launch the interactive Qt 6 GUI workspace
  rmap --export            Batch headless RTL/UVM/C/Rust/Python generation
  rmap --lint              Run automated CI register map linting (SARIF/JUnit)
  rmap --convert           Bi-directional format conversion (SVD, RDL, IP-XACT, JSON)
  rmap --diff              Semantic register map structural AST diffing

Environment Variables Configured:
  PATH                     Prepends rmap binary location
  RMAP_TEMPLATES_DIR       Path to code generation templates
  RMAP_EXAMPLES_DIR        Path to reference peripheral examples
  RMAP_DOCS_DIR            Path to local documentation portal
]])

-- Resolve package root dynamically based on module location or explicit environment
local my_module_dir = myFileName():match("(.*/)")
local base_root = os.getenv("RMAP_ROOT") or pathJoin(my_module_dir, "../..")

-- Normalize path
local pkg_root = pathNormalize(base_root)

-- Prepend executable search path
prepend_path("PATH", pathJoin(pkg_root, "bin"))

-- Shared libraries if present
if isDir(pathJoin(pkg_root, "lib")) then
    prepend_path("LD_LIBRARY_PATH", pathJoin(pkg_root, "lib"))
end

-- Resource paths
setenv("RMAP_ROOT", pkg_root)
setenv("RMAP_TEMPLATES_DIR", pathJoin(pkg_root, "share/rmap/templates"))
setenv("RMAP_EXAMPLES_DIR", pathJoin(pkg_root, "share/rmap/examples"))
setenv("RMAP_DOCS_DIR", pathJoin(pkg_root, "share/rmap/doc"))
