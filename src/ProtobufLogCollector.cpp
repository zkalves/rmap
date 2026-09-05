/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "ProtobufLogCollector.hpp"

void ProtobufLogCollector::AddError(int line, int column, const std::string& message) {
    m_str += "ERROR (" + std::to_string(line + 1) + "," + std::to_string(column + 1) + "):" + message + "\n";
}

void ProtobufLogCollector::AddWarning(int line, int column, const std::string& message) {
    m_str += "WARNING (" + std::to_string(line + 1) + "," + std::to_string(column + 1) + "):" + message + "\n";
}

