### `rtl/reg_map.vhd.inja`: Synthesizable VHDL Register File
- **Standard / Target**: IEEE 1076-1993/2008 VHDL
- **Category**: RTL Synthesizable Hardware
- **Default Deliverable Path**: `{out_dir}/{name}.vhd`
- **Description**: Synthesizable VHDL entity and architecture with standard std_logic_vector ports, generic parameterization, and synchronous process blocks.

#### Generic Parameters & Configurations

| Parameter | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `DATA_WIDTH` | `natural` | `32` | Data bus width in bits |
| `ADDR_WIDTH` | `natural` | `32` | Address bus width in bits |
| `PARAM_HW_PRECEDENCE` | `integer` | `1` | Arbitration priority |

#### Interface Signals & Ports

| Signal Name | Direction | Type / Width | Description |
| :--- | :--- | :--- | :--- |
| `clk_i` | IN | `std_logic` | Clock signal |
| `rst_ni` | IN | `std_logic` | Active-low asynchronous reset |
| `bus_addr_i` | IN | `std_logic_vector` | Address bus |
| `bus_wdata_i` | IN | `std_logic_vector` | Write data bus |
| `bus_wstrb_i` | IN | `std_logic_vector` | Byte write enables |
| `bus_wr_en_i` | IN | `std_logic` | Write strobe enable |
| `bus_rd_en_i` | IN | `std_logic` | Read strobe enable |
| `bus_rdata_o` | OUT | `std_logic_vector` | Read data bus output |
| `sw_<reg>_wr_strobe_o` | OUT | `std_logic` | Write pulse strobe output |
| `sw_<reg>_rd_strobe_o` | OUT | `std_logic` | Read pulse strobe output |

#### Protocol Timing Diagrams (WaveDrom)

##### VHDL reg_map: Software Write & Strobe Timing

![VHDL reg_map: Software Write & Strobe Timing](../../../images/wavedrom/rtl_sw_write_strobe.png)

<details><summary>WaveDrom JSON Source (Timing Specification)</summary>

```wavedrom
{
  "signal": [
    {
      "name": "clk_i",
      "wave": "p........"
    },
    {
      "name": "bus_addr_i",
      "wave": "x.=.=.x..",
      "data": [
        "0x00",
        "0x04"
      ]
    },
    {
      "name": "bus_wdata_i",
      "wave": "x.=.=.x..",
      "data": [
        "0xCAFE",
        "0xBEEF"
      ]
    },
    {
      "name": "bus_wstrb_i",
      "wave": "0.1.1.0.."
    },
    {
      "name": "bus_wr_en_i",
      "wave": "0.1.1.0.."
    },
    {
      "name": "sw_ctrl_wr_strobe_o",
      "wave": "0.1.0...."
    },
    {
      "name": "sw_status_wr_strobe_o",
      "wave": "0...1.0.."
    },
    {
      "name": "reg_ctrl_q",
      "wave": "x..=.....",
      "data": [
        "0xCAFE"
      ]
    }
  ],
  "head": {
    "text": "RTL Template: Software Write Cycle with Byte Strobes & Pulse Strobe"
  }
}
```
</details>
