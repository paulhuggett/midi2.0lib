midi2
=====

**midi2** is a C++23 library of the building blocks that MIDI 2.0 devices and
applications need. It is a collection of small, independent sub-libraries: Universal
MIDI Packet (UMP) messages and a UMP dispatcher; translators between MIDI 1.0
bytestreams, MIDI 1.0 UMP, and MIDI 2.0 UMP; MIDI Capability Inquiry (MIDI-CI) messages
and a MIDI-CI dispatcher; and the codecs that MIDI-CI uses. Use as many or as few of them
as you need.

Every part of the library processes its input one byte or one 32-bit word at a time. None
of it allocates memory or throws exceptions, and every buffer has a size that is fixed at
compile time. It is equally at home in a microcontroller and in a desktop application.

This program uses three of the sub-libraries. It converts a MIDI 1.0 bytestream to MIDI
1.0 UMP, translates that to MIDI 2.0 UMP, and dispatches the result to a handler:

.. literalinclude:: examples/home.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   note on: note=60 velocity=0xffff
   note on: note=64 velocity=0x8000

The library assumes that you know the MIDI 1.0 and MIDI 2.0 specifications. It uses
their terms throughout, and it cites their section numbers. The specifications are
available from `the MIDI Association <https://midi.org/specifications>`_.

How this documentation is organized
-----------------------------------

:doc:`introduction`
   The sub-libraries, the ideas they share, and when to use them.

:doc:`tutorial`
   A step-by-step walk-through: build and send UMP messages, dispatch received
   messages, write a handler with no run-time overhead, translate between MIDI 1.0 and
   MIDI 2.0, and answer a MIDI-CI Discovery message. Start here if you are new.

:doc:`howto/index`
   Short recipes for specific tasks, grouped by sub-library: carrying System Exclusive
   data in UMP, running MIDI-CI over UMP, receiving large Property Exchange messages,
   choosing message handlers at run time, and more.

:doc:`reference/index`
   Precise descriptions of every public type, function, and concept, one page for each
   sub-library.

.. toctree::
   :maxdepth: 2
   :hidden:

   introduction
   tutorial
   howto/index
   reference/index
