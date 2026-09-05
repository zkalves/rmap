/* This file is part of rmap.
 *
 * rmap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rmap.  If not, see <https://www.gnu.org/licenses/>.
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

