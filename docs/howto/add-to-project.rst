Add midi2 to your project
=========================

midi2 is a C++23 library. Most of it is header-only, but three of its translators have
source files, so your program links to the ``midi2`` library target. midi2 depends on
one other library, `icubaby <https://github.com/paulhuggett/icubaby>`_, which its CMake
build fetches for you.

With CMake ``FetchContent``
---------------------------

This is the easiest way. CMake downloads midi2 and icubaby when the project is
configured:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.22)
   project(my_app LANGUAGES CXX)

   include(FetchContent)
   set(MIDI2_UNITTEST No)  # Don't build midi2's own unit tests
   FetchContent_Declare(
     midi2
     GIT_REPOSITORY https://github.com/paulhuggett/midi2.0lib.git
     GIT_TAG main  # A commit hash pins the source exactly.
   )
   FetchContent_MakeAvailable(midi2)

   add_executable(my_app main.cpp)
   target_compile_features(my_app PRIVATE cxx_std_23)
   target_link_libraries(my_app PRIVATE midi2)

The ``midi2`` target carries the include paths for both libraries. Setting
``MIDI2_UNITTEST`` to ``No`` before ``FetchContent_MakeAvailable()`` stops midi2 from
downloading Google Test and building its unit tests as part of your project.
:ref:`howto/add-to-project:cmake options` lists the other settings.

With a Git submodule
--------------------

If you keep your dependencies in your own repository, add midi2 as a submodule and
include it with ``add_subdirectory()``:

.. code-block:: console

   $ git submodule add https://github.com/paulhuggett/midi2.0lib.git third_party/midi2

.. code-block:: cmake

   set(MIDI2_UNITTEST No)  # Don't build midi2's own unit tests
   add_subdirectory(third_party/midi2)
   target_link_libraries(my_app PRIVATE midi2)

midi2's own ``CMakeLists.txt`` still fetches icubaby with ``FetchContent``. To use a
copy of icubaby that you already have, set ``FETCHCONTENT_SOURCE_DIR_ICUBABY`` to its
directory when you configure.

CMake options
-------------

midi2's ``CMakeLists.txt`` has these options. Set them with ``-D`` when you configure
midi2's own build, as in ``cmake -S . -B build -D MIDI2_DEMO=Yes``, or with ``set()``
before ``FetchContent_MakeAvailable()`` or ``add_subdirectory()``.

.. list-table::
   :header-rows: 1
   :widths: 30 12 58

   * - Option
     - Default
     - Effect
   * - ``MIDI2_UNITTEST``
     - ``Yes``
     - Builds the unit tests, and downloads Google Test to do so. Set it to ``No``
       when you only want the library.
   * - ``MIDI2_DOC_EXAMPLES``
     - ``No``
     - Builds the example programs that this documentation includes, and adds a
       ``ctest`` test that runs each of them.
   * - ``MIDI2_DEMO``
     - ``No``
     - Builds the demo programs in the ``demo`` directory, and adds a ``ctest`` test
       for most of them. The demos download the
       `peejay <https://github.com/paulhuggett/peejay>`_ JSON parser and
       `zlib <https://zlib.net>`_.
   * - ``MIDI2_FUZZTEST``
     - ``No``
     - Builds the fuzz tests, and downloads `FuzzTest <https://github.com/google/fuzztest>`_
       to do so.
   * - ``MIDI2_BENCHMARK``
     - ``No``
     - Builds the benchmark. It uses `Google Benchmark <https://github.com/google/benchmark>`_,
       which is downloaded unless CMake finds an installed copy.
   * - ``MIDI2_WERROR``
     - ``No``
     - Treats compiler warnings as errors in midi2's own targets.
   * - ``MIDI2_SANITIZE``
     - ``No``
     - Builds midi2's own targets with AddressSanitizer and UndefinedBehaviorSanitizer.
       GCC and Clang only.
   * - ``MIDI2_COVERAGE``
     - ``No``
     - Builds midi2's own targets with code coverage instrumentation. GCC and Clang
       only.

None of these options changes how your own code is compiled.

Without CMake
-------------

Add the midi2 and icubaby ``include`` directories to your compiler's search path, select
C++23, and compile midi2's three source files with your program:

.. code-block:: console

   $ c++ -std=c++23 -I midi2/include -I icubaby/include \
       main.cpp midi2/src/bytestream_to_ump.cpp midi2/src/ump_to_midi1.cpp midi2/src/ump_to_midi2.cpp \
       -o my_app

If your program doesn't use :cpp:class:`midi2::bytestream::to_ump`,
:cpp:class:`midi2::ump::to_midi1`, or :cpp:class:`midi2::ump::ump_to_midi2`, you can
leave out their source files. Each file is named after its header.

Which header to include
-----------------------

``midi2/midi2.hpp`` includes everything except the MIDI-CI dispatcher. To keep build
times down, include only what you use. :ref:`reference/library:headers` lists every
header and what it provides.
