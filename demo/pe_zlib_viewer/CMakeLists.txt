#===-- CMakeLists ------------------------------------------------------------*- C++ -*-===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
#
# SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
# SPDX-License-Identifier: MIT
#
#===------------------------------------------------------------------------------------===//

add_executable(demo_pe_zlib_viewer pe_zlib_viewer.cpp mcoded7_decoder.hpp zlib_decoder.hpp)
target_link_libraries(demo_pe_zlib_viewer PRIVATE midi2 peejay::peejay ZLIB::ZLIBSTATIC)
setup_target(demo_pe_zlib_viewer)

add_test(NAME demo_pe_zlib_viewer COMMAND demo_pe_zlib_viewer)
