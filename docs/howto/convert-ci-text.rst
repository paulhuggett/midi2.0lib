Convert text to and from MIDI-CI 7-bit text
===========================================

Text in a MIDI-CI message, such as the JSON of a Property Exchange header, must be 7-bit.
Characters outside ASCII are written as JSON-style ``\uXXXX`` escapes, using a UTF-16
surrogate pair if necessary. midi2 provides this conversion as two specializations of
icubaby's ``transcoder`` class template, so they are used in the same way as icubaby's
own transcoders:

.. literalinclude:: ../examples/howto_ci7text.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   MIDI-CI text: Caf\u00E9 \\ Piano \uD83C\uDFB9
   round trip succeeded

How it works
------------

* ``midi2::transcoder<char8_t, char>`` converts UTF-8 to 7-bit text. The input type can
  also be ``char16_t`` or ``char32_t``. ASCII characters are copied, except that a
  backslash is written as ``\\``; every other code point becomes ``\uXXXX``.
* ``midi2::transcoder<char, char8_t>`` converts 7-bit text to UTF-8, ``char16_t``, or
  ``char32_t``. It decodes the JSON escapes ``\"``, ``\\``, ``\/``, ``\b``, ``\f``,
  ``\n``, ``\r``, ``\t``, and ``\uXXXX``.
* Call the transcoder with each code unit of the input and an output iterator. It
  returns the iterator past the last code unit written. When the input ends, call
  ``end_cp()``, which reports an incomplete sequence at the end of the input.
* Malformed input makes ``well_formed()`` return ``false``. The encoder replaces it
  with U+FFFD REPLACEMENT CHARACTER, which it then writes as ``\uFFFD``. When decoding,
  a byte with its most significant bit set, an unknown escape, and a malformed
  ``\uXXXX`` sequence are all malformed: :ref:`reference/codecs:midi-ci 7-bit text`
  describes what the decoder writes for each.
* The header is ``midi2/ci/ci7text.hpp``. ``midi2::transcoder`` is a ``using``
  declaration for ``icubaby::transcoder``.

The decoder recognizes the same escapes as a JSON parser, so if you pass Property
Exchange JSON straight to a JSON parser, you don't need it. It is for text that you
display or store outside JSON.
