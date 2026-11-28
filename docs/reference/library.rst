Headers, namespaces, and shared concepts
========================================

This page describes what all of the sub-libraries have in common: the headers, the
namespaces, the concepts that describe translators and dispatchers, and the
user-defined literals.

Headers
-------

Include only the headers that provide what you use. Each header includes the others
that it needs.

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Header
     - Provides
   * - ``midi2/midi2.hpp``
     - Everything below except ``midi2/ci/ci_dispatcher.hpp``,
       ``midi2/ci/ci_dispatcher_backend.hpp``, and ``midi2/mcoded7.hpp``.
   * - ``midi2/ump/ump_types.hpp``
     - The UMP message types, :cpp:func:`midi2::ump::apply`,
       :cpp:func:`midi2::ump::check`, and the :cpp:concept:`midi2::ump::ump_message`
       concept. See :doc:`ump-messages`.
   * - ``midi2/ump/ump_dispatcher.hpp``
     - :cpp:class:`midi2::ump::ump_dispatcher`, its configuration types, and the UMP
       backends. See :doc:`ump-dispatcher`.
   * - ``midi2/ump/ump_dispatcher_backend.hpp``
     - The UMP backend concepts and classes alone.
   * - ``midi2/ump/ump_to_midi2.hpp``
     - :cpp:class:`midi2::ump::ump_to_midi2`. See :doc:`ump-translators`.
   * - ``midi2/ump/ump_to_midi1.hpp``
     - :cpp:class:`midi2::ump::to_midi1`.
   * - ``midi2/ump/ump_utils.hpp``
     - :cpp:func:`midi2::ump::mcm_scale` and the :cpp:enum:`midi2::ump::control`
       enumeration.
   * - ``midi2/bytestream/bytestream_to_ump.hpp``
     - :cpp:class:`midi2::bytestream::to_ump`. See :doc:`bytestream`.
   * - ``midi2/bytestream/ump_to_bytestream.hpp``
     - :cpp:class:`midi2::bytestream::to_bytestream`.
   * - ``midi2/bytestream/usbm1_to_bytestream.hpp``
     - :cpp:class:`midi2::bytestream::usbm1_to_bytestream`.
   * - ``midi2/bytestream/bytestream_types.hpp``
     - The :cpp:enum:`midi2::bytestream::status` enumeration and functions that classify
       bytestream bytes.
   * - ``midi2/ci/ci_types.hpp``
     - The MIDI-CI message types and the ``b7``, ``b14``, and ``b28`` types. See
       :doc:`ci-messages`.
   * - ``midi2/ci/ci_create_message.hpp``
     - :cpp:func:`midi2::ci::create_message` and :cpp:struct:`midi2::ci::trivial_sentinel`.
   * - ``midi2/ci/ci_dispatcher.hpp``
     - :cpp:class:`midi2::ci::ci_dispatcher`, its configuration types, and the MIDI-CI
       backends. See :doc:`ci-dispatcher`.
   * - ``midi2/ci/ci_dispatcher_backend.hpp``
     - The MIDI-CI backend concepts and classes alone.
   * - ``midi2/ci/ci7text.hpp``
     - The transcoders between Unicode and MIDI-CI 7-bit text. See :doc:`codecs`.
   * - ``midi2/mcoded7.hpp``
     - :cpp:class:`midi2::mcoded7::encoder` and :cpp:class:`midi2::mcoded7::decoder`.
   * - ``midi2/translator.hpp``
     - The :cpp:concept:`midi2::translator` concept.
   * - ``midi2/dispatcher.hpp``
     - The :cpp:concept:`midi2::dispatcher` concept.
   * - ``midi2/utils.hpp``
     - The ``_b`` literal and small utilities.

Namespaces
----------

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Namespace
     - Contents
   * - ``midi2``
     - The :cpp:concept:`midi2::translator` and :cpp:concept:`midi2::dispatcher`
       concepts, and ``midi2::transcoder``.
   * - ``midi2::ump``
     - The UMP dispatcher and protocol translators. The message types are in nested
       namespaces named after their message types: ``utility``, ``system``, ``m1cvm``,
       ``data64``, ``m2cvm``, ``data128``, ``stream``, and ``flex_data``.
   * - ``midi2::ump::mt``
     - Enumerations of the status values of each UMP message type.
   * - ``midi2::ump::dispatcher_backend``
     - The UMP backend concepts and classes.
   * - ``midi2::bytestream``
     - The bytestream translators.
   * - ``midi2::ci``
     - The MIDI-CI message types, ``create_message()``, and the MIDI-CI dispatcher.
       Messages for Profile Configuration, Property Exchange, and Process Inquiry are in
       the nested namespaces ``profile_configuration``, ``property_exchange``, and
       ``process_inquiry``.
   * - ``midi2::ci::dispatcher_backend``
     - The MIDI-CI backend concepts and classes.
   * - ``midi2::mcoded7``
     - The Mcoded7 encoder and decoder.
   * - ``midi2::literals``
     - The ``_b``, ``_b7``, ``_b14``, and ``_b28`` user-defined literals.
   * - ``midi2::adt``
     - Fixed-capacity data structures used by the library, such as a FIFO and a cache.

The types and functions in any namespace named ``details``, and in each MIDI-CI
namespace named ``packed`` (which describe the on-the-wire layout of each message), are
implementation details. They may change without notice, and you should not use them.

Translators
-----------

.. code-block:: cpp

   #include "midi2/translator.hpp"

   namespace midi2 {
     template <typename InputType, typename OutputType, typename T>
     concept translator = /* see below */;
   }

A translator converts a stream of values of one type to a stream of values of another.
``T`` satisfies ``translator<InputType, OutputType, T>`` if, for a value ``v`` of type
``T``:

``typename T::input_type``
   is ``InputType``: the type of the input values.

``typename T::output_type``
   is ``OutputType``: the type of the output values.

``v.push(InputType{})``
   gives the translator one value of input. It may produce any number of output values,
   up to the capacity of the translator's output FIFO, or none.

``std::as_const(v).empty()``
   returns ``true`` if there is no output waiting to be popped.

``v.pop()``
   removes and returns the next value of output. ``empty()`` must be ``false``.

``v.reset()``
   restores the translator to its initial state. Any partly translated input and any
   waiting output are lost.

The library's translators are:

.. list-table::
   :header-rows: 1
   :widths: 38 20 20 22

   * - Translator
     - ``input_type``
     - ``output_type``
     - Output capacity
   * - :cpp:class:`midi2::bytestream::to_ump`
     - ``std::byte``
     - ``std::uint32_t``
     - 4 words
   * - :cpp:class:`midi2::bytestream::to_bytestream`
     - ``std::uint32_t``
     - ``std::byte``
     - 8 bytes
   * - :cpp:class:`midi2::bytestream::usbm1_to_bytestream`
     - ``std::uint32_t``
     - ``std::byte``
     - 4 bytes
   * - :cpp:class:`midi2::ump::ump_to_midi2`
     - ``std::uint32_t``
     - ``std::uint32_t``
     - 4 words
   * - :cpp:class:`midi2::ump::to_midi1`
     - ``std::uint32_t``
     - ``std::uint32_t``
     - 4 words

The capacity is enough for the output of one input value. Pop all of the output after
each call to ``push()``: output that doesn't fit is discarded.

Copying translators
^^^^^^^^^^^^^^^^^^^

.. warning::

   :cpp:class:`midi2::bytestream::to_bytestream`, :cpp:class:`midi2::ump::to_midi1`, and
   :cpp:class:`midi2::ump::ump_to_midi2` each hold a UMP dispatcher whose context is a
   pointer to the translator's own state. A copy or a moved-to object still refers to
   the state of the original, so its output appears in the original. Don't copy or move
   these three translators. Construct each one where it is to be used.

Dispatchers
-----------

.. code-block:: cpp

   #include "midi2/dispatcher.hpp"

   namespace midi2 {
     template <typename Config, typename InputType, typename T>
     concept dispatcher = /* see below */;
   }

A dispatcher accepts a stream of values, decodes them into messages, and calls the
handler for each message. ``T`` satisfies ``dispatcher<Config, InputType, T>`` if, for a
value ``v`` of type ``T``:

``typename T::input_type``
   is ``InputType``: the type of the input values.

``typename T::config_type``
   is ``Config`` with any ``std::reference_wrapper<>`` and reference removed.

``v.dispatch(InputType{})``
   gives the dispatcher one value of input.

``v.config()``
   returns a reference to the configuration. There is a ``const`` overload.

The two dispatchers are :cpp:class:`midi2::ump::ump_dispatcher` (``std::uint32_t``
input) and :cpp:class:`midi2::ci::ci_dispatcher` (``std::byte`` input). Both also have
a ``reset()`` member function, which discards a partly received message.

Literals
--------

.. code-block:: cpp

   #include "midi2/utils.hpp"      // _b
   #include "midi2/ci/ci_types.hpp"  // _b7, _b14, _b28

   namespace midi2::literals {
     consteval std::byte operator""_b(unsigned long long);
     consteval ci::b7 operator""_b7(unsigned long long);
     consteval ci::b7 operator""_b7(char);
     consteval ci::b14 operator""_b14(unsigned long long);
     consteval ci::b28 operator""_b28(unsigned long long);
   }

Each literal makes a value of its type: ``0x7E_b`` is ``std::byte{0x7E}``, and
``0x7F_b7`` is ``midi2::ci::b7{0x7F}``. ``'A'_b7`` makes a ``b7`` from a character. The
argument must fit: ``_b`` takes values up to 255, ``_b7`` up to 0x7F, ``_b14`` up to
0x3FFF, and ``_b28`` up to 0x0FFFFFFF. The literals are ``consteval``, so when
assertions are enabled, a value that doesn't fit is a compile-time error.

Preprocessor macros
-------------------

``MIDI2_TEXT_COMMON_STRING``
   If defined with a non-zero value before ``midi2/ump/ump_types.hpp`` is included,
   ``midi2::ump::flex_data::text_common`` has a ``str()`` member function that returns its
   text as a ``std::u8string``. ``str()`` allocates memory, so it is off by default.
