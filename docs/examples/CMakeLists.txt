#===-- docs/examples/CMakeLists.txt ------------------------------------------*- C++ -*-===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
#
# SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
# SPDX-License-Identifier: MIT
#
#===------------------------------------------------------------------------------------===//
# Builds the example programs that are included in the documentation and runs each of them as a test so that the
# documentation stays correct.

set(midi2_doc_examples
  home
  howto_backend_base
  howto_bytestream_to_ump
  howto_ci7text
  howto_ci_over_ump
  howto_context
  howto_mcoded7
  howto_pe
  howto_protocol
  howto_scale
  howto_sysex7
  howto_ump_to_bytestream
  howto_usbm1
  tutorial_1_messages
  tutorial_2_dispatch
  tutorial_3_backend
  tutorial_4_translate
  tutorial_5_ci
)
foreach(example ${midi2_doc_examples})
  add_executable(doc_${example} ${example}.cpp)
  target_link_libraries(doc_${example} PRIVATE midi2)
  setup_target(doc_${example})
  add_test(NAME doc_${example} COMMAND doc_${example})
endforeach()
