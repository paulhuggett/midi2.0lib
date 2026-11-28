MIDI-CI messages
================

.. code-block:: cpp

   #include "midi2/ci/ci_types.hpp"
   #include "midi2/ci/ci_create_message.hpp"

   namespace midi2::ci {
     using b7 = /* 7-bit unsigned integer */;
     using b14 = /* 14-bit unsigned integer */;
     using b28 = /* 28-bit unsigned integer */;
     using muid = b28;
     constexpr muid max_user_muid = muid{0x0FFFFEFF};
     constexpr muid broadcast_muid = muid{0x0FFFFFFF};
     template <std::size_t Size> using b7_array = std::array<b7, Size>;
     template <std::size_t Size> using byte_array = std::array<std::byte, Size>;

     enum class message : std::uint8_t;
     struct header;
     // ... one struct for each message (see below) ...

     template <typename T> concept ci_message = /* see below */;
     struct trivial_sentinel;
     template </* see below */> constexpr O create_message(O first, S last, header const& hdr, T const& t);
   }

Each MIDI-CI message is a struct whose members are the fields of the message as defined
by M2-101 (MIDI Capability Inquiry), with the values decoded: a multi-byte field such as
a MUID or a 14-bit number is a single integer, not a sequence of 7-bit bytes.

Integer types
-------------

``b7``, ``b14``, and ``b28`` hold unsigned integers of 7, 14, and 28 bits. Each is a
specialization of the class template ``midi2::ci::details::bn<Bits>``, with these
members:

``constexpr bn() noexcept``
   Constructs 0.

``template <std::unsigned_integral UInt> constexpr explicit bn(UInt v) noexcept``
   Constructs ``v``. When assertions are enabled, ``v`` must fit in ``Bits`` bits.

``constexpr Underlying get() const noexcept`` and ``constexpr explicit operator Underlying() const noexcept``
   Return the value. ``Underlying`` is the smallest standard unsigned type with at least
   ``Bits`` bits.

``constexpr bn& operator=(Underlying v) noexcept``
   Assigns ``v``, which must fit.

``operator<=>``, ``operator==``
   Compare two values of the same type.

The literals ``_b7``, ``_b14``, and ``_b28`` make values of these types: see
:ref:`reference/library:literals`.

The message header
------------------

``struct header``

   The fields that every MIDI-CI message has (M2-101 section 5.2.1):

   ``b7 device_id``
      The Device ID: 0x00 to 0x0F for a MIDI channel, 0x7E for the group, or 0x7F for the
      function block. The default is 0.

   ``b7 version``
      The MIDI-CI Message Version/Format. The default is 1.

   ``muid remote_muid``
      The Source MUID: the MUID of the device that sent the message.

   ``muid local_muid``
      The Destination MUID: the MUID of the device to which the message is sent. Use
      ``broadcast_muid`` for a broadcast message.

The names are those of the receiver: when you receive a message, ``remote_muid`` is the
other device's MUID and ``local_muid`` is yours. When you send one, ``remote_muid`` is
yours.

Message types
-------------

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Namespace
     - Structs
   * - ``midi2::ci``
     - ``discovery``, ``discovery_reply``, ``endpoint``, ``endpoint_reply``,
       ``invalidate_muid``, ``ack``, ``nak``
   * - ``midi2::ci::profile_configuration``
     - ``inquiry``, ``inquiry_reply``, ``added``, ``removed``, ``details``,
       ``details_reply``, ``on``, ``off``, ``enabled``, ``disabled``,
       ``specific_data``
   * - ``midi2::ci::property_exchange``
     - ``capabilities``, ``capabilities_reply``, ``get``, ``get_reply``, ``set``,
       ``set_reply``, ``subscription``, ``subscription_reply``, ``notify``
   * - ``midi2::ci::process_inquiry``
     - ``capabilities``, ``capabilities_reply``, ``midi_message_report``,
       ``midi_message_report_reply``, ``midi_message_report_end``

``enum class message : std::uint8_t`` gives the Sub-ID#2 value of each message type, for
example ``message::discovery`` (0x70) and ``message::pe_get_reply`` (0x35).
``profile_configuration::profile`` is ``b7_array<5>``, a Profile ID.

Each struct is an aggregate whose members can be set with designated initializers. Every
struct has ``operator==``. Most also have:

``static constexpr T make(packed::... const&)``
   Decode a message from its on-the-wire layout. The dispatcher uses these: you don't need
   them.

``explicit constexpr operator packed::...() const noexcept``
   Encode the message in its on-the-wire layout. ``create_message()`` uses these.

Where M2-101 defines different layouts for version 1 and version 2 of a message, the
struct has the fields of the later version. Fields that version 1 doesn't have are
ignored when a version 1 message is written, and are 0 (or a value passed to ``make()``)
when one is read.

Variable-length data
^^^^^^^^^^^^^^^^^^^^

Some fields are a sequence whose length is given by another field in the message. The
struct holds a ``std::span`` that refers to memory it doesn't own, and the length is the
span's size:

.. list-table::
   :header-rows: 1
   :widths: 45 55

   * - Struct
     - Members
   * - ``endpoint_reply``
     - ``std::span<b7 const> information``
   * - ``ack``, ``nak``
     - ``std::span<b7 const> message``: the message text
   * - ``profile_configuration::inquiry_reply``
     - ``std::span<profile const> enabled``, ``std::span<profile const> disabled``
   * - ``profile_configuration::details_reply``, ``profile_configuration::specific_data``
     - ``std::span<b7 const> data``
   * - ``property_exchange::get`` and the other six Property Exchange data messages
     - ``std::span<char const> header``, ``std::span<std::byte const> data``

When you write a message, the memory must remain valid until ``create_message()``
returns. When the dispatcher passes you a message, the spans refer to the dispatcher's
buffer, and are valid only until the handler returns.

Property Exchange data messages
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``template <property_exchange_type Pet> class property_exchange::property_exchange``

   The seven messages that carry a header and property data are specializations of this
   template: ``get``, ``get_reply``, ``set``, ``set_reply``, ``subscription``,
   ``subscription_reply``, and ``notify``. Its members are:

   ``chunk_info chunk``
      ``b14 number_of_chunks`` (0 if unknown) and ``b14 chunk_number`` (from 1).

   ``b7 request``
      The Request ID.

   ``std::span<char const> header``
      The header data: JSON text.

   ``std::span<std::byte const> data``
      The property data. It is JSON text unless the header's ``mutualEncoding`` or
      ``mediaType`` property says otherwise.

   ``static constexpr property_exchange make(chunk_info const& chunk, b7 request, std::span<char const> header, std::span<std::byte const> data = {}) noexcept``
      Returns a message with these members.

``property_exchange::stream_start`` and ``property_exchange::stream_chunk`` are passed
to a ``property_exchange_stream`` backend: see :doc:`ci-dispatcher`.

Writing messages
----------------

``template <typename T> concept ci_message``
   Satisfied by every message struct listed above.

``create_message(O first, S last, header const& hdr, T const& t)``
   *Requires:* ``T`` satisfies ``ci_message``, ``O`` is an output iterator for
   ``std::byte``, and ``S`` is a sentinel for ``O``.

   Writes the message ``t``, with the header ``hdr``, to the range ``[first, last)``, and
   returns an iterator past the last byte written. The bytes start with the Universal
   System Exclusive ID (0x7E) and don't include ``F0`` or ``F7``. The Sub-ID#2 byte and
   any length fields are filled in from ``T`` and its spans.

   ``hdr.version`` selects the layout: 1 selects the version 1 layout, and any other value
   the version 2 layout. A message that M2-101 doesn't define for version 1 (for example,
   the Process Inquiry messages) is not written at all with version 1.

   A message is written in parts: the header, then the fixed part of the body, then each
   variable-length part. A part is written only if all of it fits. If the message doesn't
   fit, some parts may have been written, and the iterator returned is ``last``. A message
   that exactly fills the range also returns ``last``, so to tell the two apart, use a
   range at least one byte larger than the largest message you write.

``struct trivial_sentinel``
   A sentinel that is never equal to an iterator. Use it as ``last`` with an output
   iterator that can't run out of space, such as a ``std::back_insert_iterator``.
