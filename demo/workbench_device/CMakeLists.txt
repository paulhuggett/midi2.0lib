#===-- CMakeLists ------------------------------------------------------------*- C++ -*-===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
#
# SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
# SPDX-License-Identifier: MIT
#
#===------------------------------------------------------------------------------------===//

# A MIDI 2.0 device that the MIDI 2.0 Workbench can connect to. It uses CoreMIDI so is built only on macOS. It runs
# until interrupted, so it is not registered as a test.
add_executable(demo_workbench_device workbench_device.cpp)
target_link_libraries(demo_workbench_device PRIVATE midi2 peejay::peejay "-framework CoreMIDI" "-framework CoreFoundation")
setup_target(demo_workbench_device)
