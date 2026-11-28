Welcome to midi2!
=================

The AM MIDI 2.0 Library is a general-purpose C++ library that provides the building blocks, processing, and translations needed for MIDI 2.0 devices and applications. It enables bidirectional conversion between MIDI 1.0 bytestreams and Universal MIDI Packet (UMP) format, supports processing and constructing UMP streams, and handles MIDI Capability Inquiry (MIDI-CI) messages.

Installation
------------

The library needs a C++23 compiler and `CMake <https://cmake.org>`_. Add it to a CMake project with ``add_subdirectory()`` or ``FetchContent``, then link your target with the ``midi2`` library:

.. code-block:: cmake

    include(FetchContent)
    FetchContent_Declare(midi2 GIT_REPOSITORY https://github.com/paulhuggett/midi2.0lib.git GIT_TAG main)
    FetchContent_MakeAvailable(midi2)
    target_link_libraries(my_program PRIVATE midi2)

CMake fetches the library's one dependency, `icubaby <https://github.com/paulhuggett/icubaby>`_, automatically. The library's unit tests are built by default; set the ``MIDI2_UNITTEST`` option to ``No`` before ``FetchContent_MakeAvailable()`` to skip them.

Usage
-----

Include ``<midi2/midi2.hpp>`` for the translators, the UMP message types and dispatcher, and MIDI-CI message creation. The MIDI-CI dispatcher is in ``<midi2/ci/ci_dispatcher.hpp>``.

The `README <https://github.com/paulhuggett/midi2.0lib#readme>`_ has short examples, and the `demo <https://github.com/paulhuggett/midi2.0lib/tree/main/demo>`_ directory has complete programs.
