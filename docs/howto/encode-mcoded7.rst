Encode and decode Mcoded7
=========================

MIDI-CI messages can carry only 7-bit bytes. Mcoded7, described in M2-103 (Common Rules
for MIDI-CI Property Exchange), encodes 8-bit data by sending the most significant bits
of each group of seven bytes in an extra byte before them. Property Exchange uses it
when a header's ``mutualEncoding`` property is ``"Mcoded7"`` or ``"zlib+Mcoded7"``.
:cpp:class:`midi2::mcoded7::encoder` and :cpp:class:`midi2::mcoded7::decoder` convert
one byte at a time:

.. literalinclude:: ../examples/howto_mcoded7.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   original: 81 02 FF 04 05 06 87 F0
   encoded:  51 01 02 7F 04 05 06 07 40 70
   decoded:  81 02 FF 04 05 06 87 F0
   valid Mcoded7

How it works
------------

* ``parse_byte(value, out)`` takes one byte of input and writes any output to the output
  iterator ``out``. It returns the iterator past the last byte written: keep it and pass
  it to the next call.
* The encoder writes nothing until it has a group of seven bytes, then writes all eight
  bytes at once (``encoder::max_size``). The decoder writes at most one byte for each byte
  of input (``decoder::max_size``).
* ``flush(out)`` must be called at the end of the input. The encoder writes the last,
  short, group. The decoder has nothing to write, but call it anyway so that the two can
  be used in the same way.
* ``good()`` returns ``false`` if the decoder has seen a byte with its most significant
  bit set, which can't appear in Mcoded7. The encoder's ``good()`` always returns
  ``true``.
* ``reset()`` prepares an encoder or decoder for a new sequence of data.
