Introduction
============

midi2 is a C++23 library for MIDI 2.0. Rather than a single large component, it is a
collection of sub-libraries, each of which does one job and can be used on its own. This
page describes the sub-libraries, the ideas that they share, and how to decide whether
the library suits your program.

The sub-libraries
-----------------

.. list-table::
   :header-rows: 1
   :widths: 20 28 52

   * - Sub-library
     - Namespace and header
     - What it provides
   * - :doc:`UMP messages <reference/ump-messages>`
     - ``midi2::ump``

       ``midi2/ump/ump_types.hpp``
     - A type for every Universal MIDI Packet message, with an accessor for each field.
       :cpp:func:`midi2::ump::apply` passes the words of a message to your transport.
   * - :doc:`UMP dispatcher <reference/ump-dispatcher>`
     - ``midi2::ump``

       ``midi2/ump/ump_dispatcher.hpp``
     - :cpp:class:`midi2::ump::ump_dispatcher` assembles incoming 32-bit words into
       messages and calls a handler for each one.
   * - :doc:`Protocol translators <reference/ump-translators>`
     - ``midi2::ump``

       ``midi2/ump/ump_to_midi2.hpp``, ``midi2/ump/ump_to_midi1.hpp``
     - Translate MIDI 1.0 Protocol UMP messages to MIDI 2.0 Protocol messages and back,
       following the "Translation" appendix of M2-104.
   * - :doc:`Bytestream <reference/bytestream>`
     - ``midi2::bytestream``

       ``midi2/bytestream/*.hpp``
     - Translate a MIDI 1.0 bytestream to UMP and back, and USB-MIDI 1.0 Event Packets
       to a bytestream.
   * - :doc:`MIDI-CI messages <reference/ci-messages>`
     - ``midi2::ci``

       ``midi2/ci/ci_types.hpp``, ``midi2/ci/ci_create_message.hpp``
     - A type for the body of every MIDI-CI message, and
       :cpp:func:`midi2::ci::create_message` to serialize one.
   * - :doc:`MIDI-CI dispatcher <reference/ci-dispatcher>`
     - ``midi2::ci``

       ``midi2/ci/ci_dispatcher.hpp``
     - :cpp:class:`midi2::ci::ci_dispatcher` decodes the bytes of a MIDI-CI System
       Exclusive message and calls a handler for it. Property Exchange messages can be
       received in pieces, so they may be larger than the dispatcher's buffer.
   * - :doc:`Codecs <reference/codecs>`
     - ``midi2::mcoded7``, ``midi2``

       ``midi2/mcoded7.hpp``, ``midi2/ci/ci7text.hpp``
     - Mcoded7 encoding and decoding, and the transcoders between Unicode and the
       7-bit text of MIDI-CI Property Exchange.

``midi2/midi2.hpp`` includes all of these except the MIDI-CI dispatcher.
:doc:`reference/library` lists every header.

The sub-libraries are layered. The UMP message types are the foundation of almost
everything else: the UMP dispatcher decodes words into these types, and the translators
that read UMP are themselves built on the dispatcher. The MIDI-CI sub-libraries don't
depend on UMP at all. MIDI-CI messages are System Exclusive messages, so you can carry
them over UMP, a MIDI 1.0 bytestream, or any other transport.

.. graphviz::

   digraph layers {
     rankdir=BT;
     node [shape=box, fontname="sans-serif", fontsize=11];
     edge [color="#888888"];
     ump [label="UMP messages"];
     dispatcher [label="UMP dispatcher"];
     protocol [label="Protocol translators"];
     bytestream [label="Bytestream"];
     cimsg [label="MIDI-CI messages"];
     cidisp [label="MIDI-CI dispatcher"];
     codecs [label="Codecs"];
     dispatcher -> ump;
     protocol -> dispatcher;
     bytestream -> dispatcher;
     cidisp -> cimsg;
   }

Key ideas
---------

Message types
   Each message has its own type. A UMP message type, such as
   ``midi2::ump::m2cvm::note_on``, holds the words of the message and has a "getter"
   and a chainable "setter" for each field, so you never write a shift or a mask. A
   MIDI-CI message type, such as :cpp:struct:`midi2::ci::discovery`, is an aggregate
   whose members hold the fields of the message body. The 7-, 14-, and 28-bit fields of
   MIDI-CI messages have the types :cpp:type:`midi2::ci::b7`, :cpp:type:`midi2::ci::b14`,
   and :cpp:type:`midi2::ci::b28`, which can't hold a value that is out of range.

Translators
   A **translator** converts one stream of values into another: bytes to UMP words,
   MIDI 1.0 UMP to MIDI 2.0 UMP, and so on. Every translator has the same four member
   functions. ``push()`` gives it one value of input; ``empty()`` says whether output is
   waiting; ``pop()`` takes one value of output; and ``reset()`` returns it to its
   initial state. The :cpp:concept:`midi2::translator` concept describes them. Because
   the interface is the same, translators can be chained, and code can be written once
   for any of them.

Dispatchers
   A **dispatcher** accepts its input one value at a time through ``dispatch()``. When
   the input completes a message, the dispatcher decodes it and calls a **handler**
   function that you supply. There are two: :cpp:class:`midi2::ump::ump_dispatcher`
   accepts 32-bit UMP words, and :cpp:class:`midi2::ci::ci_dispatcher` accepts the bytes
   of MIDI-CI messages.

Configurations, backends, and contexts
   A dispatcher's template argument is its **configuration**: a type whose members
   provide the handlers. The handlers are gathered into **backends**, one for each group
   of related messages, such as the MIDI 2.0 Channel Voice messages or the MIDI-CI
   Property Exchange messages. The configuration also holds a **context**: a value that
   the dispatcher passes by reference to every handler, so that the handlers can share
   state with each other and with the rest of your program.

Choosing backends
   The library provides four kinds of backend for every group of messages. A **null**
   backend ignores its messages, and because its handlers are ``static constexpr``
   functions that do nothing, the compiler removes the code that leads up to them. A
   **function** backend holds a ``std::function<>`` for each message and is the easiest
   to use. The **pure** and **base** backends declare virtual functions, so you can
   override just the handlers you need, or choose the handlers at run time. And any type
   with the right member functions can be a backend: a backend whose handlers are static
   functions costs nothing more than the work those functions do.

Design goals
------------

No dynamic memory allocation
   The library never allocates memory. Every buffer, FIFO, and cache has a size that is
   fixed at compile time. A dispatcher or translator is an ordinary object that you can
   put on the stack, in static storage, or inside your own objects. (The ``function``
   backends hold ``std::function<>`` objects, which may allocate when you install a
   handler.)

No exceptions
   The library does not throw or catch exceptions. Errors in the input, such as a
   message that is too large or an unknown message type, are reported to a handler or
   discarded, as described for each component.

Small footprint
   Input is processed one byte or one word at a time, and messages are decoded in
   place, so the code and data needed for a simple MIDI 2.0 device are small enough for
   a microcontroller.

The type system, not magic numbers
   Message layouts are described by types rather than by shifts and masks in your code.
   Out-of-range field values are caught by assertions in debug builds.

Zero-overhead dispatch
   Handlers are passed as template arguments rather than function pointers, so the
   compiler can inline them and optimize across the call.

When to use midi2
-----------------

midi2 is a good fit when:

* you are writing a MIDI 2.0 device or application and want the protocol details
  handled for you, but want to keep control of the transport and of what is stored;
* your target has no heap, or you can't let the arrival of MIDI data allocate memory;
* your code is built without exceptions;
* you need to bridge MIDI 1.0 bytestreams or MIDI 1.0 Protocol devices to MIDI 2.0.

It is up to your program to:

* send and receive the bytes or words, over whatever transport you use;
* store the details of the remote MIDI-CI devices that you discover;
* interpret the data carried by MIDI-CI messages, such as Profile IDs and Property
  Exchange JSON. (The demo programs use the `peejay <https://github.com/paulhuggett/peejay>`_
  JSON parser for this.);
* decide when to send a MIDI-CI NAK, and handle the NAKs that it receives.

Requirements
------------

* A C++23 compiler and standard library. Continuous integration tests Apple Clang
  (macOS 15), GCC 13, Clang 18, and MSVC (Visual Studio 2022).
* `CMake <https://cmake.org>`_ 3.12 or later.
* `icubaby <https://github.com/paulhuggett/icubaby>`_, a small header-only Unicode
  library that the MIDI-CI text transcoders use. The CMake build fetches it
  automatically. See :doc:`howto/add-to-project`.

Most of the library is header-only, but three translators
(:cpp:class:`midi2::bytestream::to_ump`, :cpp:class:`midi2::ump::ump_to_midi2`, and
:cpp:class:`midi2::ump::to_midi1`) have source files, so you link to the ``midi2``
library target.

Things to know
--------------

A few behaviours are worth knowing before you start. Each is covered in more detail in
the reference.

* Drain a translator after every ``push()``. A translator's output FIFO holds only as
  much as one input value can produce (at most eight bytes or four words). Output that
  doesn't fit is lost.
* The spans in a received MIDI-CI message, such as the header and property data of a
  Property Exchange message, refer to the dispatcher's buffer. They are valid only until
  the handler returns, so copy anything you need to keep.
* Call :cpp:func:`midi2::ci::ci_dispatcher::reset` at the start of every System
  Exclusive message. Once a MIDI-CI message has been dispatched, the dispatcher discards
  any further bytes until it is reset.
* A MIDI-CI message is dispatched only if it is addressed to the broadcast MUID or if
  the configuration's ``check_muid()`` handler accepts its destination MUID. The default
  handlers reject every MUID, so install your own.
* The ``group`` and ``channel`` fields of UMP messages are numbered from 0, as they are
  on the wire. MIDI 1.0 "channel 1" is channel 0.
* Don't copy or move :cpp:class:`midi2::bytestream::to_bytestream`,
  :cpp:class:`midi2::ump::to_midi1`, or :cpp:class:`midi2::ump::ump_to_midi2`. See
  :ref:`reference/library:copying translators`.
