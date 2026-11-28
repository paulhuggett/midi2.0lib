Scale values between resolutions
================================

MIDI 2.0 messages have more resolution than their MIDI 1.0 equivalents: 16 bits for a
velocity instead of 7, and 32 bits for a controller value instead of 7 or 14. When you
convert a value yourself, use :cpp:func:`midi2::ump::mcm_scale`, which implements the
"min-center-max" algorithm of section 3 of M2-115 (MIDI 2.0 Bit Scaling and Resolution).
The translators use the same function.

.. literalinclude:: ../examples/howto_scale.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   7-bit 0x00 -> 16-bit 0x0000
   7-bit 0x01 -> 16-bit 0x0200
   7-bit 0x40 -> 16-bit 0x8000
   7-bit 0x41 -> 16-bit 0x8208
   7-bit 0x7f -> 16-bit 0xffff
   16-bit 0x0000 -> 7-bit 0x00
   16-bit 0x8000 -> 7-bit 0x40
   16-bit 0x81ff -> 7-bit 0x40
   16-bit 0xffff -> 7-bit 0x7f

How it works
------------

* The template arguments are the number of bits in the source value and in the result.
  The source must have more than one bit, and the result at most 32. The argument and
  result types are the smallest unsigned integer types that hold that many bits.
* When scaling up, the minimum, center, and maximum values map to the minimum, center,
  and maximum of the result. Values up to the center are shifted left; values above it
  have their lower bits filled by repeating the bits below the most significant bit, so
  that the maximum maps to the maximum.
* When scaling down, the least significant bits are discarded.
* The function is ``constexpr``, so it can be evaluated at compile time.
