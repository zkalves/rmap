### `sim/Makefile.inja`: Multi-Tool Simulation Makefile
- **Standard / Target**: GNU Make
- **Category**: Format Interchange & Verification
- **Default Deliverable Path**: `{out_dir}/Makefile`
- **Description**: Automated test runner targeting Icarus Verilog (sim-rtl), Verilator (sim-verilator), Cocotb/pyuvm (sim-pyuvm), and Verilator / commercial EDA suites (sim-uvm SIM=verilator|vcs|xrun|mti), with configurable waveform dumping (`DUMP=1` / `TRACE=1` / `WAVE=1`).