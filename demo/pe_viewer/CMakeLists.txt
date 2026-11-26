#===-- CMakeLists ------------------------------------------------------------*- C++ -*-===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
#
# SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
# SPDX-License-Identifier: MIT
#
#===------------------------------------------------------------------------------------===//

add_executable(demo_pe_viewer pe_viewer.cpp)
target_link_libraries(demo_pe_viewer PRIVATE midi2 peejay::peejay)
setup_target(demo_pe_viewer)

add_test(NAME demo_pe_viewer COMMAND demo_pe_viewer)
