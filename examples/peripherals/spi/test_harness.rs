// This file is part of rmap.
//
// rmap is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// rmap is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with rmap.  If not, see <https://www.gnu.org/licenses/>.
//
// Copyright (c) 2026 Ezequiel Alves. All rights reserved.

#[path = "work/rust/reg_map.rs"]
mod reg_map;

use reg_map::{CTRL_REG, STATUS_REG};

fn main() {
    println!("[Rust Harness] Testing SPI PAC Definitions...");
    assert_eq!(CTRL_REG::OFFSET, 0x0000);
    assert_eq!(STATUS_REG::OFFSET, 0x0004);
    assert_eq!(CTRL_REG::EN_MASK, 0x1);
    assert_eq!(CTRL_REG::MODE_MASK, 0x6);

    let val = CTRL_REG::set_en(0, 1);
    assert_eq!(CTRL_REG::get_en(val), 1);

    let val2 = CTRL_REG::set_mode(val, 3);
    assert_eq!(CTRL_REG::get_mode(val2), 3);
    println!("[Rust Harness] All SPI assertions PASSED!");
}
