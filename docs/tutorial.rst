Tutorial
========

This tutorial builds five small programs. Each one introduces one part of midi2, and
each part can be used without the others:

#. :ref:`tutorial:build and send ump messages`, using the UMP message types.
#. :ref:`tutorial:receive ump messages`, using the UMP dispatcher.
#. :ref:`tutorial:write a handler with no overhead`.
#. :ref:`tutorial:translate between midi 1.0 and midi 2.0`, using the translators.
#. :ref:`tutorial:answer a midi-ci discovery message`, using the MIDI-CI messages and
   dispatcher.

You need a C++23 compiler and CMake 3.12 or later. The tutorial assumes that you know
the UMP format and MIDI-CI, and takes about half an hour. The complete source of every
program is in the ``docs/examples`` directory of the midi2 repository.

Set up a project
----------------

Create an empty directory with this ``CMakeLists.txt``:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.22)
   project(midi2_tutorial LANGUAGES CXX)

   include(FetchContent)
   set(MIDI2_UNITTEST No)  # Don't build midi2's own unit tests
   FetchContent_Declare(
     midi2
     GIT_REPOSITORY https://github.com/paulhuggett/midi2.0lib.git
     GIT_TAG main
   )
   FetchContent_MakeAvailable(midi2)

   add_executable(messages messages.cpp)
   target_compile_features(messages PRIVATE cxx_std_23)
   target_link_libraries(messages PRIVATE midi2)

Linking to the ``midi2`` target sets the include paths for midi2 and for icubaby, the
Unicode library it depends on. As you add the programs below, add an
``add_executable()`` line and matching ``target_compile_features()`` and
``target_link_libraries()`` lines for each one.

Build and send UMP messages
---------------------------

Every UMP message has its own type in a namespace named after its message type:
``midi2::ump::m2cvm`` for the MIDI 2.0 Channel Voice messages, ``midi2::ump::data64``
for the 64-bit data messages, and so on. Save this as ``messages.cpp``:

.. literalinclude:: examples/tutorial_1_messages.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Build and run it:

.. code-block:: console

   $ cmake -S . -B build && cmake --build build
   $ ./build/messages
   note=60 velocity=0xc000
   2 words: 40923C00 C0000000
   2 words: 40924000 C0000000
   w0=40924000 w1=C0000000
   1 word: 20C20500

Here is what the program does.

``midi2::ump::m2cvm::note_on{}``
   A default-constructed message has its message type and status fields set, and every
   other field set to zero. The message type and status can't be changed.

``.group(0).channel(2).note(60).velocity(0xC000)``
   Each field has a setter, named after the field, which returns ``*this`` so that the
   calls can be chained. The getter of the same name, which takes no arguments, returns
   the field's value. Your code never needs to know which word holds a field, or
   where. A value that doesn't fit in its field is caught by an assertion.

``midi2::ump::apply(non, transmit)``
   Calls ``transmit()`` once for each 32-bit word of the message, starting with the
   first. The function can return any type that converts to ``bool``. If it returns a
   value that converts to ``true``, such as a ``std::error_code`` that holds an error,
   ``apply()`` stops and returns that value.

``std::tuple_size_v`` and ``auto const [w0, w1] = non``
   The message types are tuple-like: their elements are the words of the message. A
   word converts explicitly to ``std::uint32_t``. Most code never needs this, because
   ``apply()`` does the work.

Receive UMP messages
--------------------

A :cpp:class:`midi2::ump::ump_dispatcher` does the opposite job. You give it 32-bit
words, one at a time, and when the words make up a complete message, it calls the
handler for that message. This program receives the words that the first program sent:

.. literalinclude:: examples/tutorial_2_dispatch.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   note on:  channel=2 note=60 velocity=0xc000
   note on:  channel=2 note=64 velocity=0xc000
   program change: channel=2 program=5
   note off: channel=2 note=64
   unknown message: 60000000
   4 messages handled

Some points to notice:

* :cpp:func:`make_ump_function_dispatcher() <midi2::ump::make_ump_function_dispatcher>`
  creates a dispatcher whose handlers are ``std::function<>`` objects. Its argument is
  the **context**: a value that the dispatcher stores, and passes by reference to every
  handler. Here, the context counts the messages that were handled. The program reads
  the count through ``dispatcher.config().context``.
* ``dispatcher.config()`` returns the dispatcher's **configuration**. It has one member
  for each group of messages, named after the namespace that holds the message types:
  ``utility``, ``system``, ``m1cvm``, ``data64``, ``m2cvm``, ``data128``, ``stream``,
  and ``flex``. Each one has an ``on_...()`` function for each message, which installs
  a handler and returns the member, so that the calls can be chained.
* A message without a handler is ignored, like the UMP Stream message here.
* A message whose message type or status is reserved is passed to the
  ``utility.unknown()`` handler as a span of its words.
* The handler receives the message as an object of its message type, so it reads the
  fields with the same getters that you used to build it.

.. note::

   ``std::function<>`` may allocate memory, and calls through it can't be inlined. The
   ``function`` handlers are the easiest to use, but the next program shows how to
   avoid that cost.

Write a handler with no overhead
--------------------------------

A dispatcher's configuration can be any type that has a ``context`` member and the
eight members that provide the handlers, which are called **backends**. Each backend
must have a member function for each message in its group. The library provides a
``_null`` backend for every group, whose handlers are ``static constexpr`` functions
that do nothing. Derive from one, and replace the functions for the messages that you
want. This program records which notes are held:

.. literalinclude:: examples/tutorial_3_backend.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   2 notes held on channel 2: 60 67

Here is how it works.

``note_tracker``
   Derives from :cpp:struct:`m2cvm_null\<held_notes> <midi2::ump::dispatcher_backend::m2cvm_null>`
   and hides its ``note_on()`` and ``note_off()`` functions with its own. Each handler
   takes the context by reference and the message by const reference.

``config``
   Has the ``context`` and one backend for each group of messages. The backends that
   ignore everything are empty, and ``[[no_unique_address]]`` stops them from taking any
   space. :cpp:concept:`midi2::ump::ump_dispatcher_config` checks the configuration at
   compile time: if a backend lacks a handler, or a handler has the wrong signature, the
   compiler names the requirement that isn't met.

``midi2::ump::ump_dispatcher dispatcher{config{}}``
   The class template argument is deduced from the configuration. The dispatcher calls
   ``note_tracker::note_on()`` directly, so the compiler can inline it. All of the
   messages in the other groups lead to functions that do nothing, so the compiler can
   remove the code that decodes them.

The library also provides ``_base`` backends, which declare a virtual function for every
handler: :doc:`howto/choose-handlers-at-run-time` shows how to use them.

Translate between MIDI 1.0 and MIDI 2.0
---------------------------------------

The translators convert one form of MIDI into another. They all have the same
interface: ``push()`` gives a translator one value of input, ``empty()`` says whether
any output is waiting, ``pop()`` takes one value of output, and ``reset()`` returns it to
its initial state. This program passes a MIDI 1.0 bytestream through four translators,
to MIDI 2.0 Protocol UMP and back again:

.. literalinclude:: examples/tutorial_4_translate.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   MIDI 1.0 bytestream:  90 3C 7F 40 40 B0 07 64
   MIDI 1.0 UMP:         20903C7F 20904040 20B00764
   MIDI 2.0 UMP:         40903C00 FFFF0000 40904000 80000000 40B00700 C9249249
   MIDI 1.0 UMP:         20903C7F 20904040 20B00764
   MIDI 1.0 bytestream:  90 3C 7F 40 40 B0 07 64

Some points to notice:

* ``translate()`` works with any translator. The :cpp:concept:`midi2::translator`
  concept describes the interface, and each translator's ``input_type`` and
  ``output_type`` name the types of its input and output.
* Drain each translator after every ``push()``. A translator holds only as much output
  as one input value can produce. Output that doesn't fit is lost.
* :cpp:class:`midi2::bytestream::to_ump` handles running status: the second Note On
  has no status byte. Its constructor argument is the UMP group of the messages that it
  produces.
* :cpp:class:`midi2::ump::ump_to_midi2` scales each value up, using the min-center-max
  algorithm of M2-115 (MIDI 2.0 Bit Scaling and Resolution), so a velocity of 0x7F
  becomes 0xFFFF and 0x40 becomes 0x8000. :cpp:class:`midi2::ump::to_midi1` scales
  values down again. Bank Select and RPN/NRPN Control Change sequences are converted to
  the MIDI 2.0 Program Change and Registered and Assignable Controller messages:
  :doc:`howto/translate-midi1-and-midi2` shows how.
* :cpp:class:`midi2::bytestream::to_bytestream` uses running status where it can.

Answer a MIDI-CI Discovery message
----------------------------------

The MIDI-CI sub-libraries work with the bytes of MIDI-CI System Exclusive messages,
without the ``F0`` and ``F7`` bytes that start and end them. A
:cpp:class:`midi2::ci::ci_dispatcher` decodes a message one byte at a time, and
:cpp:func:`midi2::ci::create_message` writes one. This program receives a Discovery
message and builds the Reply to Discovery:

.. literalinclude:: examples/tutorial_5_ci.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   Discovery from MUID 0x567890: manufacturer 12 23 34, max SysEx size 256
   Reply: 7E 7F 0D 71 02 43 65 06 05 10 71 59 02 7D 00 00 01 00 02 00 01 00 00 00 0C 00 04 00 00 00 00

Here is how it works.

``make_function_dispatcher<context, 256>(0x7F_b7, std::uint8_t{0})``
   Creates a dispatcher whose handlers are ``std::function<>`` objects. Its template
   arguments are the type of the context and the size of the buffer that holds the body
   of each message: a message that doesn't fit is reported to the ``buffer_overflow()``
   handler. The function arguments are the Device ID that the dispatcher records in the
   header passed to each handler, and the UMP group that it passes to
   ``check_muid()``.

``on_check_muid()``
   The dispatcher calls this handler with the Destination MUID of every message that
   isn't sent to the broadcast MUID. If it returns ``false``, the message is ignored.
   The default handler rejects every message, so you almost always install one.

``on_discovery()``
   Called with the message's :cpp:struct:`midi2::ci::header` and its
   :cpp:struct:`midi2::ci::discovery` body. In the header, ``remote_muid`` is the
   Source MUID and ``local_muid`` is the Destination MUID.

``dispatcher.reset()``
   Prepares the dispatcher for a new message. Once a message has been dispatched, the
   dispatcher ignores any further bytes until it is reset, so call ``reset()`` at the
   start of each System Exclusive message.

``create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{}, header, reply)``
   Writes the header and the body to an output range, given as an iterator and a
   sentinel, and returns an iterator past the last byte written. The ``version`` field of
   the header selects the layout of the body: version 1 messages have fewer fields. A
   ``std::back_insert_iterator`` can't run out of space, so ``trivial_sentinel`` stands
   for the end of the range.

``0x7F_b7``, ``0x0001_b14``, and ``512_b28``
   The fields of MIDI-CI messages are 7-, 14-, and 28-bit values with the types
   :cpp:type:`midi2::ci::b7`, :cpp:type:`midi2::ci::b14`, and
   :cpp:type:`midi2::ci::b28`. These user-defined literals, from namespace
   ``midi2::literals``, make them. When assertions are enabled, a literal that doesn't
   fit is a compile-time error.

The bytes of the reply would normally be sent as a System Exclusive message. Over UMP,
that means a sequence of SysEx7 packets: :doc:`howto/carry-midi-ci-over-ump` shows how.

Next steps
----------

* Browse the :doc:`how-to guides <howto/index>` for recipes that solve common problems.
* Read :doc:`reference/ump-dispatcher` and :doc:`reference/ci-dispatcher` for the
  complete list of handlers and the exact order in which they are called.
* Read :doc:`reference/library` for the headers to include and the concepts shared by
  every sub-library.
