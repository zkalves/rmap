# typed: false
# frozen_string_literal: true

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

class Rmap < Formula
  desc "Hardware Register Map Designer & Model Generator"
  homepage "https://github.com/zkalves/rmap"
  url "https://github.com/zkalves/rmap/archive/refs/tags/v0.2.0.tar.gz"
  # sha256 "..."
  license "MPL-2.0"
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
