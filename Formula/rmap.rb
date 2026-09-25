# typed: false
# frozen_string_literal: true

# This file is part of rmap.
#
# rmap is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rmap is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rmap.  If not, see <https://www.gnu.org/licenses/>.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

class Rmap < Formula
  desc "Hardware Register Map Designer & Model Generator"
  homepage "https://github.com/zkalves/rmap"
  url "https://github.com/zkalves/rmap/archive/refs/tags/v0.2.0.tar.gz"
  # sha256 "..."
  license "GPL-3.0-or-later"
  head "https://github.com/zkalves/rmap.git", branch: "main"

  depends_on "cmake" => :build
  depends_on "protobuf"
  depends_on "qt@6"

  def install
    qt6 = Formula["qt@6"]

    args = std_cmake_args + %W[
      -DCMAKE_BUILD_TYPE=Release
      -DBUILD_TESTING=OFF
      -DCMAKE_PREFIX_PATH=#{qt6.opt_prefix}
      -DQT_DIR=#{qt6.opt_lib}/cmake/Qt6
    ]

    system "cmake", "-S", ".", "-B", "build", *args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    ENV["QT_QPA_PLATFORM"] = "offscreen"
    assert_match "rmap", shell_output("#{bin}/rmap --version")
  end
end
