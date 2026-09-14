// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
