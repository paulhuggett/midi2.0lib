MIDI-CI dispatcher
==================

.. code-block:: cpp

   #include "midi2/ci/ci_dispatcher.hpp"

   namespace midi2::ci {
     template <typename T> concept ci_dispatcher_config = /* see below */;
     template <typename T> concept ci_dispatcher_stream_config = /* see below */;

     template <typename Config>
       requires ci_dispatcher_config<std::unwrap_reference_t<Config>>
     class ci_dispatcher;

     template <typename Context, std::size_t BufferSize> struct function_config;
     template <typename Context, std::size_t BufferSize> struct function_stream_config;

     template <typename Context, std::size_t BufferSize>
     ci_dispatcher<function_config<Context, BufferSize>>
     make_function_dispatcher(b7 device_id, std::uint8_t group, Context&& context = Context{});
     template <typename Context, std::size_t BufferSize>
     ci_dispatcher<function_config<Context, BufferSize>> make_function_dispatcher();
   }

   #include "midi2/ci/ci_dispatcher_backend.hpp"

   namespace midi2::ci::dispatcher_backend {
     // For each Group in system, management, profile, property_exchange,
     // property_exchange_stream, process_inquiry:
     template <typename T, typename Context> concept Group = /* see below */;
     template <typename Context> struct Group_null;
     template <typename Context> struct Group_pure;
     template <typename Context> struct Group_base;
     template <typename Context> class Group_function;
   }

The MIDI-CI dispatcher has the same design as the :doc:`UMP dispatcher <ump-dispatcher>`:
a configuration holds a context and a backend for each group of messages, and the
dispatcher calls a backend's handler, passing the context by reference, for each message.

``ci_dispatcher``
-----------------

``template <typename Config> class ci_dispatcher``

   Accepts the bytes of one MIDI-CI message, decodes it, and calls the handler for it. It
   satisfies :cpp:concept:`midi2::dispatcher`.

   ``Config`` is a type that satisfies ``ci_dispatcher_config``, or a
   ``std::reference_wrapper`` of one.

   The dispatcher holds a buffer of ``config_type::buffer_size`` bytes, in which it
   assembles the fixed part of the message body and any variable-length data.

Member types and constants
^^^^^^^^^^^^^^^^^^^^^^^^^^

``input_type``
   ``std::byte``.

``config_type``
   ``Config`` with any ``std::reference_wrapper`` removed.

``static constexpr bool pe_stream``
   ``true`` if ``config_type`` satisfies ``ci_dispatcher_stream_config``.

Construction
^^^^^^^^^^^^

``constexpr ci_dispatcher() noexcept``
   Constructs a dispatcher with a Device ID and group of 0 and a value-initialized
   configuration.

``template <typename OtherConfig> constexpr explicit ci_dispatcher(OtherConfig&& config)``
   Constructs a dispatcher with a Device ID and group of 0.

``template <typename OtherConfig> constexpr explicit ci_dispatcher(b7 device_id, std::uint8_t group, OtherConfig&& config)``
   Constructs a dispatcher with the given Device ID and group.

``template <typename... Args> constexpr explicit ci_dispatcher(std::in_place_t, Args&&... args)``
   Constructs a dispatcher with a Device ID and group of 0, initializing the
   configuration from ``std::in_place, std::forward<Args>(args)...``.

``template <typename... Args> constexpr explicit ci_dispatcher(b7 device_id, std::uint8_t group, std::in_place_t, Args&&... args)``
   As above, with the given Device ID and group.

A deduction guide deduces ``ci_dispatcher<C>`` from ``(b7, std::uint8_t, C)``.

Members
^^^^^^^

``void dispatch(std::byte b)``
   Adds ``b`` to the message. The first byte of a message is the Universal System
   Exclusive ID (0x7E) that follows ``F0``; don't pass ``F0`` or ``F7``. When the bytes
   received so far complete the message, its handler is called. After that, every byte is
   discarded until ``reset()`` is called.

``constexpr void reset()``
   Prepares the dispatcher for a new message, abandoning any message that is partly
   received. If a streamed Property Exchange message is abandoned, the
   ``property_exchange_stream`` backend's ``abort()`` handler is called. The Device ID,
   group, and configuration are unchanged. Call ``reset()`` at the start of each System
   Exclusive message. It is ``noexcept`` unless ``pe_stream`` is ``true``.

``constexpr void set_device_id(b7 device_id) noexcept``
   Sets the Device ID that the dispatcher puts in the header that it passes to handlers.

``constexpr void set_group(std::uint8_t group) noexcept``
   Sets the group that the dispatcher passes to ``check_muid()``.

``config_type& config() noexcept`` and ``config_type const& config() const noexcept``
   Return the configuration.

Processing a message
^^^^^^^^^^^^^^^^^^^^

#. The dispatcher collects the 13 bytes of the header, and decodes the message type,
   version, Source MUID, and Destination MUID. The header that it passes to handlers has
   the dispatcher's own Device ID, not the one in the message, and the Universal System
   Exclusive ID and Sub-ID#1 bytes aren't checked.
#. If the message type isn't one that the library knows, it calls
   ``system.unknown_midici(context, header)`` and discards the rest of the message.
#. If the Destination MUID isn't ``broadcast_muid``, it calls
   ``system.check_muid(context, group, destination_muid)``. If that returns ``false``,
   the rest of the message is discarded.
#. It collects the fixed part of the message body, using the version 1 layout if the
   header's version is 1 and the version 2 layout otherwise, then any variable-length
   data.
#. If the message doesn't fit in the buffer, it calls ``system.buffer_overflow(context)``
   and discards the rest of the message.
#. Otherwise, it calls the handler for the message with the context, the header, and the
   message. The spans in the message refer to the dispatcher's buffer: they are valid
   only until the handler returns.

If ``pe_stream`` is ``true``, the seven Property Exchange messages that carry header and
property data are passed to the ``property_exchange_stream`` backend in pieces, instead,
as described in :doc:`../howto/exchange-property-data`. Each piece is at most
``buffer_size`` bytes, so these messages can be of any size.

``ci_dispatcher_config``
------------------------

``template <typename T> concept ci_dispatcher_config``

   Satisfied by a type with these members, where ``Context`` is
   ``decltype((v.context))`` for a value ``v`` of type ``T``:

   .. list-table::
      :header-rows: 1
      :widths: 35 65

      * - Member
        - Requirement
      * - ``buffer_size``
        - A constant expression convertible to ``std::size_t``, at least 13 (the size
          of the header).
      * - ``context``
        - Any type.
      * - ``system``
        - ``dispatcher_backend::system<Context>``
      * - ``management``
        - ``dispatcher_backend::management<Context>``
      * - ``profile``
        - ``dispatcher_backend::profile<Context>``
      * - ``property_exchange``
        - ``dispatcher_backend::property_exchange<Context>``
      * - ``process_inquiry``
        - ``dispatcher_backend::process_inquiry<Context>``

``template <typename T> concept ci_dispatcher_stream_config``

   Satisfied by a type that satisfies ``ci_dispatcher_config`` and also has a member
   ``property_exchange_stream`` that satisfies
   ``dispatcher_backend::property_exchange_stream<Context>``.

Backend concepts
----------------

Each handler must be callable with an lvalue of the context type, the
:cpp:struct:`header <midi2::ci::header>`, and (for most handlers) the message, and must
return ``void``, except for ``check_muid()``.

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Backend
     - Handlers
   * - ``system``
     - ``check_muid(context, std::uint8_t group, muid destination)``, which returns a
       value convertible to ``bool``; ``unknown_midici(context, header)``;
       ``buffer_overflow(context)``
   * - ``management``
     - ``discovery``, ``discovery_reply``, ``endpoint``, ``endpoint_reply``,
       ``invalidate_muid``, ``ack``, ``nak``
   * - ``profile``
     - ``inquiry(context, header)``, ``inquiry_reply``, ``added``, ``removed``,
       ``details``, ``details_reply``, ``on``, ``off``, ``enabled``, ``disabled``,
       ``specific_data``
   * - ``property_exchange``
     - ``capabilities``, ``capabilities_reply``, ``get``, ``get_reply``, ``set``,
       ``set_reply``, ``subscription``, ``subscription_reply``, ``notify``
   * - ``property_exchange_stream``
     - ``start(context, header, property_exchange::stream_start)``,
       ``header_data(context, header, std::span<char const>)``,
       ``chunk(context, header, property_exchange::stream_chunk)``,
       ``property_data(context, header, std::span<std::byte const>)``,
       ``end(context, header)``, ``abort(context, header)``
   * - ``process_inquiry``
     - ``capabilities(context, header)``, ``capabilities_reply``,
       ``midi_message_report``, ``midi_message_report_reply``,
       ``midi_message_report_end(context, header)``

Each handler takes the message struct of the same name in the namespace for its group:
for example, ``profile.on()`` takes a ``profile_configuration::on``. The handlers whose
messages have no fields beyond the header take only the context and the header, as
shown.

Backend classes
---------------

As for the UMP dispatcher, each backend has four class templates:

``Group_null<Context>``
   Every handler is ``static`` and does nothing. ``system_null::check_muid()`` returns
   ``false``.

``Group_pure<Context>``
   Every handler is a pure virtual function.

``Group_base<Context>``
   Derived from ``Group_pure<Context>``; every handler is a virtual function that does
   nothing. ``system_base::check_muid()`` returns ``false``.

``Group_function<Context>``
   Stores a ``std::function`` for each handler, set with ``on_handler(f)``, for example
   ``on_discovery(f)``. Each returns ``*this``. A message whose function is empty is
   ignored. If no function is set for ``check_muid``, ``check_muid()`` returns ``false``.

Because the default ``check_muid()`` returns ``false``, a dispatcher that doesn't
provide one receives only broadcast messages.

``function_config`` and ``function_stream_config``
--------------------------------------------------

``template <typename Context, std::size_t BufferSize> struct function_config``

   A configuration whose backends are the five ``_function`` classes: members
   ``system``, ``management``, ``profile``, ``property_exchange``, and
   ``process_inquiry``. ``buffer_size`` is ``BufferSize``. ``context`` is of type
   ``Context`` and declared ``[[no_unique_address]]``.

``constexpr explicit function_config(Context c = Context{})``
   Moves ``c`` into ``context``.

``template <typename... Args> constexpr explicit function_config(std::in_place_t, Args&&... args)``
   Constructs ``context`` from ``std::forward<Args>(args)...``.

``template <typename Context, std::size_t BufferSize> struct function_stream_config``

   Derived from ``function_config<Context, BufferSize>``, with the same constructors and a
   ``property_exchange_stream`` member of type
   ``dispatcher_backend::property_exchange_stream_function<Context>``.

``make_function_dispatcher<Context, BufferSize>(b7 device_id, std::uint8_t group, Context&& context = Context{})``
   Returns a dispatcher with the given Device ID and group, whose configuration is a
   ``function_config<Context, BufferSize>`` initialized from
   ``std::forward<Context>(context)``. ``Context`` and ``BufferSize`` must be given
   explicitly.

``make_function_dispatcher<Context, BufferSize>()``
   Returns a dispatcher with a Device ID and group of 0 and a value-initialized context.
