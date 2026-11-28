Codecs
======

Mcoded7
-------

.. code-block:: cpp

   #include "midi2/mcoded7.hpp"

   namespace midi2::mcoded7 {
     class encoder;
     class decoder;
   }

``encoder`` converts a sequence of 8-bit bytes to Mcoded7, and ``decoder`` converts it
back. Mcoded7 is defined by M2-103 (Common Rules for MIDI-CI Property Exchange): each
group of up to seven bytes is preceded by a byte that holds the most significant bit of
each of them, so that every byte of output is 7-bit. Both classes have the same members:

``static constexpr std::size_t max_size``
   The largest number of bytes that one call to ``parse_byte()`` writes: 8 for the
   encoder, 1 for the decoder.

``encoder()``, ``decoder() noexcept``
   Construct an encoder or decoder ready for a new sequence.

``template <std::output_iterator<std::byte> OutputIterator> OutputIterator parse_byte(std::byte value, OutputIterator out)``
   Gives the next byte of input. Writes any output to ``out`` and returns an iterator
   past the last byte written.

   The encoder writes nothing until it has seven bytes, then writes the eight bytes of
   the group. The decoder writes nothing for the first byte of each group of eight, and
   one byte for each of the others.

``template <std::output_iterator<std::byte> OutputIterator> OutputIterator flush(OutputIterator out)``
   Call at the end of the input. The encoder writes the last group if it is incomplete,
   then resets itself. The decoder writes nothing.

``bool good() const``
   For the decoder, ``false`` if a byte of input had its most significant bit set, which
   can't occur in Mcoded7. Always ``true`` for the encoder.

``void reset()``
   Discards any partial group and prepares for a new sequence. For the decoder, it also
   makes ``good()`` return ``true`` again.

See :doc:`../howto/encode-mcoded7`.

MIDI-CI 7-bit text
------------------

.. code-block:: cpp

   #include "midi2/ci/ci7text.hpp"

   namespace icubaby {
     template <unicode_char_type InputEncoding> class transcoder<InputEncoding, char>;
     template <unicode_char_type OutputEncoding> class transcoder<char, OutputEncoding>;
   }
   namespace midi2 {
     using icubaby::transcoder;
   }

These are partial specializations of the
`icubaby <https://github.com/paulhuggett/icubaby>`_ ``transcoder`` class template, in
which ``char`` stands for the 7-bit text of MIDI-CI messages. ``unicode_char_type`` is
``char8_t``, ``char16_t``, or ``char32_t``. The interface is icubaby's:

``input_type``, ``output_type``
   The code-unit types of the input and output.

``OutputIterator operator()(input_type code_unit, OutputIterator dest) noexcept``
   Gives the next code unit of input. Writes any output to ``dest`` and returns an
   iterator past the last code unit written.

``OutputIterator end_cp(OutputIterator dest)``
   Call at the end of the input. It reports input that ended part-way through a code
   point or an escape sequence: see below.

``bool well_formed() const noexcept``
   ``false`` if any of the input was malformed.

``bool partial() const noexcept``
   ``true`` if the transcoder is part-way through a code point.

``transcoder<InputEncoding, char>`` encodes Unicode as 7-bit text. A code point from
U+0000 to U+007F is written as itself, except that a backslash is written as ``\\``.
Every other code point is written as ``\uXXXX``, with four uppercase hexadecimal
digits, or as two such escapes (a UTF-16 surrogate pair) if it is above U+FFFF.
Malformed input is replaced with U+FFFD, which is then written as ``\uFFFD``.

``transcoder<char, OutputEncoding>`` decodes 7-bit text. It recognizes the escapes of
JSON strings: ``\"``, ``\\``, ``\/``, ``\b``, ``\f``, ``\n``, ``\r``, ``\t``, and
``\uXXXX`` (including surrogate pairs). Every other character is written as itself.
Malformed input makes ``well_formed()`` return ``false``, and is handled like this:

* A byte with its most significant bit set is discarded.
* In an unknown escape, such as ``\q``, the backslash is discarded and the character is
  written as itself.
* A ``\u`` escape with a character that isn't a hexadecimal digit is discarded,
  including that character.
* An unpaired surrogate is replaced with U+FFFD. This is reported by ``well_formed()``
  only after ``end_cp()`` has been called.
* At the end of the input, ``end_cp()`` writes U+FFFD for an unpaired high surrogate,
  and discards an incomplete escape.

``partial()`` always returns ``false`` for the decoder.

See :doc:`../howto/convert-ci-text`.
