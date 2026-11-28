UMP messages
============

.. code-block:: cpp

   #include "midi2/ump/ump_types.hpp"

   namespace midi2::ump {
     enum class message_type : std::uint8_t;
     template <typename T> concept ump_message = /* see below */;

     template <ump_message T, std::invocable<std::uint32_t> Function>
     constexpr std::invoke_result_t<Function, std::uint32_t> apply(T const& message, Function function);
     template <ump_message T> constexpr bool check(T const& message);

     template <message_type> struct message_size;
     template <typename T> struct max_size;
     template <typename T> inline constexpr std::size_t max_size_v = max_size<T>::value;
   }

Each UMP message is a class whose name is the message's name in M2-104 (Universal MIDI
Packet and MIDI 2.0 Protocol), in a namespace named after its message type.

Message types
-------------

.. list-table::
   :header-rows: 1
   :widths: 10 22 68

   * - MT
     - Namespace
     - Classes
   * - 0x0
     - ``utility``
     - ``noop``, ``jr_clock``, ``jr_timestamp``, ``delta_clockstamp_tpqn``,
       ``delta_clockstamp``
   * - 0x1
     - ``system``
     - ``midi_time_code``, ``song_position_pointer``, ``song_select``,
       ``tune_request``, ``timing_clock``, ``sequence_start``, ``sequence_continue``,
       ``sequence_stop``, ``active_sensing``, ``reset``
   * - 0x2
     - ``m1cvm``
     - ``note_off``, ``note_on``, ``poly_pressure``, ``control_change``,
       ``program_change``, ``channel_pressure``, ``pitch_bend``
   * - 0x3
     - ``data64``
     - ``sysex7_in_1``, ``sysex7_start``, ``sysex7_continue``, ``sysex7_end``
   * - 0x4
     - ``m2cvm``
     - ``note_off``, ``note_on``, ``poly_pressure``, ``rpn_per_note_controller``,
       ``nrpn_per_note_controller``, ``rpn_controller``, ``nrpn_controller``,
       ``rpn_relative_controller``, ``nrpn_relative_controller``,
       ``per_note_management``, ``control_change``, ``program_change``,
       ``channel_pressure``, ``pitch_bend``, ``per_note_pitch_bend``
   * - 0x5
     - ``data128``
     - ``sysex8_in_1``, ``sysex8_start``, ``sysex8_continue``, ``sysex8_end``,
       ``mds_header``, ``mds_payload``
   * - 0xD
     - ``flex_data``
     - ``set_tempo``, ``set_time_signature``, ``set_metronome``, ``set_key_signature``,
       ``set_chord_name``, ``text_common``
   * - 0xF
     - ``stream``
     - ``endpoint_discovery``, ``endpoint_info_notification``,
       ``device_identity_notification``, ``endpoint_name_notification``,
       ``product_instance_id_notification``, ``jr_configuration_request``,
       ``jr_configuration_notification``, ``function_block_discovery``,
       ``function_block_info_notification``, ``function_block_name_notification``,
       ``start_of_clip``, ``end_of_clip``

``enum class message_type : std::uint8_t`` has one member for each of these message
types, with the same name as the namespace and the MT value. The enumerations in namespace
``midi2::ump::mt`` (``mt::utility``, ``mt::system_crt``, ``mt::m1cvm``, ``mt::data64``,
``mt::m2cvm``, ``mt::data128``, ``mt::flex_data``, and ``mt::stream``) give the status
values within each message type.

The structure of a message class
--------------------------------

Every message class has the same structure. This section uses
``midi2::ump::m2cvm::note_on`` as the example.

Words
^^^^^

A message is a sequence of one to four 32-bit words. Each word has a nested class:
``note_on::word0`` and ``note_on::word1``. A word class declares each field of the word
as a nested ``adt::bit_range<Index, Bits>`` type, for example ``word0::channel`` and
``word1::velocity``, and has:

* a default constructor. For ``word0``, it sets the ``mt`` and ``status`` fields to the
  values that identify the message, and every other field to 0. Every other word is 0.
* a constructor that takes the word's value as a ``std::uint32_t``.
* ``explicit operator std::uint32_t()``, which returns the word's value.
* ``bool check() const noexcept``, which returns ``true`` if the fields that identify the
  message hold the expected values. For a word other than ``word0``, ``check()`` usually
  returns ``true``.

Construction and comparison
^^^^^^^^^^^^^^^^^^^^^^^^^^^

``constexpr note_on() noexcept``
   Constructs a message whose fields that identify it are set, and whose other fields
   are 0.

``constexpr explicit note_on(std::span<std::uint32_t, 2> m) noexcept``
   Constructs a message from its words. The span's extent is the number of words in the
   message. When assertions are enabled, ``word0`` must identify the message.

``friend constexpr bool operator==(note_on const&, note_on const&) noexcept``
   Returns ``true`` if every word of the two messages is equal.

Messages are regular value types: they can be copied, assigned, and compared.

Fields
^^^^^^

For each field of each word there is a getter, ``constexpr auto field() const noexcept``,
which returns the value of the field. A field whose value is fixed, such as ``mt`` and
``status``, has only a getter. Every other field also has a setter:

.. code-block:: cpp

   constexpr note_on& field(adt::uinteger_t<Bits> v) noexcept;

``adt::uinteger_t<Bits>`` is the smallest standard unsigned integer type that has at
least ``Bits`` bits. When assertions are enabled, ``v`` must fit in ``Bits`` bits. The
setter returns ``*this``, so calls can be chained::

   auto const m = midi2::ump::m2cvm::note_on{}.group(0).channel(3).note(60).velocity(0x8000);

A field whose value is an enumeration has a getter that returns the enumeration, a
getter named ``field_raw()`` that returns the integer value, and a setter that takes the
enumeration.

Tuple interface
^^^^^^^^^^^^^^^

A message is a tuple-like type whose elements are its words:
``std::tuple_size_v<note_on>`` is the number of words, ``std::tuple_element_t<I,
note_on>`` is the type of word ``I``, and ``get<I>(m)``, found by argument-dependent
lookup, returns a reference to word ``I``. So a message can be decomposed with a
structured binding::

   auto const& [w0, w1] = m;

``message_size<message_type::m2cvm>::value`` is the number of words in each message of
that message type.

Messages that contain arrays
----------------------------

Some messages carry a sequence of values: the data bytes of the SysEx7 and SysEx8
messages, the text of ``flex_data::text_common``, the names in
``stream::endpoint_name_notification`` and ``stream::function_block_name_notification``,
and the Product Instance ID in ``stream::product_instance_id_notification``. These
classes also behave as containers of fixed capacity:

``value_type``, ``size_type``, ``difference_type``, ``iterator``, ``const_iterator``
   ``value_type`` is the type of one value, such as ``adt::uinteger_t<7>`` for SysEx7
   data. The iterators are random-access iterators.

``begin()``, ``end()``, ``cbegin()``, ``cend()``
   The range of values held by the message.

``constexpr size_type max_size() const noexcept``
   The capacity: 6 for SysEx7, 13 for SysEx8 and ``function_block_name_notification``,
   12 for ``text_common``, and 14 for ``endpoint_name_notification`` and
   ``product_instance_id_notification``.

``size()``, ``empty()``
   The number of values held. For the SysEx messages, this is the ``number_of_bytes``
   field. For text, it counts up to the first zero value.

``operator[](std::size_t i)``
   Returns a proxy object through which value ``i`` can be read and assigned.
   ``i`` must be less than ``max_size()``.

``data(std::size_t i)`` and ``data(std::size_t i, value_type v)``
   Read and write value ``i``.

``data(std::initializer_list<T>)``, ``data(Range&& range, Proj proj = {})``, ``data(I first, S last)``
   Copy at most ``max_size()`` values into the message, after applying the projection
   ``proj`` if it is given. For the SysEx messages, ``number_of_bytes`` is set to the
   number of values copied; for the others, any unused values are set to 0. They return
   ``*this``.

``midi2::ump::max_size<T>::value`` (and ``max_size_v<T>``) is ``T{}.max_size()`` for
these types, as a constant expression. For other types, ``max_size<T>`` has no
``value`` member.

``data64::sysex7_message<T>`` and ``data128::sysex8_message<T>`` are concepts satisfied
by the four SysEx7 and four SysEx8 message types.

``flex_data::text_common`` also has ``data(std::u8string_view)``, and, if
``MIDI2_TEXT_COMMON_STRING`` is enabled, ``str()``. See
:ref:`reference/library:preprocessor macros`.

Functions
---------

``template <typename T> concept ump_message``
   Satisfied by a tuple-like type of one to four elements, each of which is a UMP word
   class. Every message class satisfies it.

``apply(T const& message, Function function)``
   *Requires:* ``T`` satisfies ``ump_message``, ``Function`` can be called with a
   ``std::uint32_t``, and its result can be converted to ``bool``.

   Calls ``function`` with each word of ``message``, in order. If a call returns a value
   that converts to ``true``, no more words are passed and that value is returned.
   Otherwise, the value returned by the last call is returned. This is how you send a
   message: write each word to the output.

``check(T const& message)``
   *Requires:* ``T`` satisfies ``ump_message``.

   Returns ``true`` if ``check()`` returns ``true`` for every word of ``message``.

``mcm_scale``
-------------

.. code-block:: cpp

   #include "midi2/ump/ump_utils.hpp"

   namespace midi2::ump {
     template <unsigned SourceBits, unsigned DestBits>
       requires(SourceBits > 1 && DestBits <= 32)
     constexpr adt::uinteger_t<DestBits> mcm_scale(adt::uinteger_t<SourceBits> value) noexcept;
   }

Scales ``value`` from ``SourceBits`` bits to ``DestBits`` bits with the
"min-center-max" algorithm of M2-115 (MIDI 2.0 Bit Scaling and Resolution). When scaling
up, 0 stays 0, the center value (``1 << (SourceBits - 1)``) and values below it are
shifted, and the bits of larger values are repeated, so that the maximum becomes the
maximum. When scaling down, the value is shifted right. See :doc:`../howto/scale-values`.

``ump_utils.hpp`` also defines ``enum class control : std::uint8_t``, which names the
MIDI 1.0 controller numbers that the protocol translators interpret: ``bank_select``,
``bank_select_lsb``, ``data_entry_msb``, ``data_entry_lsb``, ``rpn_msb``, ``rpn_lsb``,
``nrpn_msb``, ``nrpn_lsb``, and ``reset_all_controllers``.
