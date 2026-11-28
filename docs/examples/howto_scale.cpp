// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <cstdint>
#include <format>
#include <iostream>

#include "midi2/ump/ump_utils.hpp"

int main() {
  // Upscale 7-bit values to 16 bits using the min-center-max algorithm.
  for (auto const v :
       {std::uint8_t{0x00}, std::uint8_t{0x01}, std::uint8_t{0x40}, std::uint8_t{0x41}, std::uint8_t{0x7F}}) {
    std::cout << std::format("7-bit {:#04x} -> 16-bit {:#06x}\n", v, midi2::ump::mcm_scale<7, 16>(v));
  }
  // Downscaling discards the least significant bits.
  for (auto const v : {std::uint16_t{0x0000}, std::uint16_t{0x8000}, std::uint16_t{0x81FF}, std::uint16_t{0xFFFF}}) {
    std::cout << std::format("16-bit {:#06x} -> 7-bit {:#04x}\n", v, midi2::ump::mcm_scale<16, 7>(v));
  }
}
