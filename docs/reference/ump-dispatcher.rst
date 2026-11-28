UMP dispatcher
==============

.. code-block:: cpp

   #include "midi2/ump/ump_dispatcher.hpp"

   namespace midi2::ump {
     template <typename T> concept ump_dispatcher_config = /* see below */;

     template <typename Config>
       requires ump_dispatcher_config<std::unwrap_reference_t<Config>>
     class ump_dispatcher;

     template <typename Context> struct function_config;

     template <typename Context>
     ump_dispatcher<function_config<Context>> make_ump_function_dispatcher(Context&& context = Context{});

     constexpr unsigned ump_message_size(message_type mt);
   }

   #include "midi2/ump/ump_dispatcher_backend.hpp"

   namespace midi2::ump::dispatcher_backend {
     // For each Group in utility, system, m1cvm, data64, m2cvm, data128, stream, flex_data:
     template <typename T, typename Context> concept Group = /* see below */;
     template <typename Context> struct Group_null;
     template <typename Context> struct Group_pure;
     template <typename Context> struct Group_base;
     template <typename Context> class Group_function;
   }

``ump_dispatcher``
------------------

``template <typename Config> class ump_dispatcher``

   Accepts a stream of 32-bit UMP words, assembles them into messages, and calls the
   handler in ``Config`` for each message. It satisfies
   :cpp:concept:`midi2::dispatcher`.

   ``Config`` is a type that satisfies ``ump_dispatcher_config``, or a
   ``std::reference_wrapper`` of one. With a ``std::reference_wrapper``, the dispatcher
   refers to a configuration that you own, which must outlive it.

Member types
^^^^^^^^^^^^

``input_type``
   ``std::uint32_t``.

``config_type``
   ``Config`` with any ``std::reference_wrapper`` removed.

Construction
^^^^^^^^^^^^

``template <typename OtherConfig> constexpr explicit ump_dispatcher(OtherConfig&& config)``
   *Requires:* ``OtherConfig`` converts to ``Config`` and isn't ``ump_dispatcher``.

   Initializes the configuration from ``std::forward<OtherConfig>(config)``. Pass
   ``std::ref(cfg)`` to refer to ``cfg``.

``template <typename... Args> constexpr explicit ump_dispatcher(std::in_place_t, Args&&... args)``
   *Requires:* ``Config`` can be constructed from ``std::in_place, args...``.

   Initializes the configuration from ``std::in_place, std::forward<Args>(args)...``. For
   :cpp:struct:`function_config <midi2::ump::function_config>`, this constructs the
   context in place, so the context type need not be copyable or movable.

A deduction guide deduces ``ump_dispatcher<T>`` from an argument of type ``T``. A
dispatcher can be copied and moved if its configuration can.

Members
^^^^^^^

``void dispatch(std::uint32_t ump)``
   Adds ``ump`` to the message being assembled. The message type of the message's first
   word gives the number of words in the message. When the message is complete, the
   handler for it is called and the next word begins a new message.

``void reset()``
   Discards a partly assembled message.

``config_type& config() noexcept`` and ``config_type const& config() const noexcept``
   Return the configuration.

Calling handlers
^^^^^^^^^^^^^^^^

For each complete message, the dispatcher calls one member function of one of the
configuration's backends, passing ``config().context`` by reference and the message as
an object of the message class. For example, a MIDI 2.0 Note On message is passed to
``config().m2cvm.note_on(config().context, m)``, where ``m`` is a
``midi2::ump::m2cvm::note_on``. The handler names are listed below.

These messages are passed to ``config().utility.unknown(context, words)``, where
``words`` is a ``std::span<std::uint32_t>`` of the message's words:

* a message with a reserved message type (0x6 to 0xC, and 0xE);
* a message with a reserved or unknown status within a known message type;
* a Flex Data message with a status bank of 0 and a status that isn't one of the five
  listed below.

A Flex Data message with a status bank other than 0 is passed to
``config().flex.text(context, m)`` as a ``flex_data::text_common``.

The dispatcher doesn't check any other field. In particular, it doesn't filter by group,
and it doesn't assemble a SysEx or Stream message that spans several packets: each
packet is passed on as it arrives.

``ump_dispatcher_config``
-------------------------

``template <typename T> concept ump_dispatcher_config``

   Satisfied by a type with a member ``context`` and eight backend members. For a value
   ``v`` of type ``T``, let ``Context`` be ``decltype((v.context))``: an lvalue reference to
   the context, ``const`` if ``T`` is. Then:

   .. list-table::
      :header-rows: 1
      :widths: 30 70

      * - Member
        - Must satisfy
      * - ``v.utility``
        - ``dispatcher_backend::utility<Context>``
      * - ``v.system``
        - ``dispatcher_backend::system<Context>``
      * - ``v.m1cvm``
        - ``dispatcher_backend::m1cvm<Context>``
      * - ``v.data64``
        - ``dispatcher_backend::data64<Context>``
      * - ``v.m2cvm``
        - ``dispatcher_backend::m2cvm<Context>``
      * - ``v.data128``
        - ``dispatcher_backend::data128<Context>``
      * - ``v.stream``
        - ``dispatcher_backend::stream<Context>``
      * - ``v.flex``
        - ``dispatcher_backend::flex_data<Context>``

The members can be of any type that satisfies the concept: a class of your own, one of
the classes below, a reference to either, or an empty class declared
``[[no_unique_address]]``. The context can be any type, including a pointer or a
reference.

Backend concepts
----------------

Each backend concept requires member functions that can be called with an lvalue of the
context type and an object of a message class, and return ``void``. The handlers in
``utility`` are:

``noop(context)``, ``jr_clock(context, m)``, ``jr_timestamp(context, m)``, ``delta_clockstamp_tpqn(context, m)``, ``delta_clockstamp(context, m)``
   The Utility messages.

``unknown(context, std::span<std::uint32_t> words)``
   A message that the dispatcher doesn't recognize, as described above.

The other backends have one handler for each message of their message type:

.. list-table::
   :header-rows: 1
   :widths: 18 82

   * - Backend
     - Handlers
   * - ``system``
     - ``midi_time_code``, ``song_position_pointer``, ``song_select``,
       ``tune_request``, ``timing_clock``, ``seq_start``, ``seq_continue``,
       ``seq_stop``, ``active_sensing``, ``reset``
   * - ``m1cvm``
     - ``note_off``, ``note_on``, ``poly_pressure``, ``control_change``,
       ``program_change``, ``channel_pressure``, ``pitch_bend``
   * - ``data64``
     - ``sysex7_in_1``, ``sysex7_start``, ``sysex7_continue``, ``sysex7_end``
   * - ``m2cvm``
     - ``note_off``, ``note_on``, ``poly_pressure``, ``program_change``,
       ``channel_pressure``, ``rpn_per_note_controller``,
       ``nrpn_per_note_controller``, ``rpn_controller``, ``nrpn_controller``,
       ``rpn_relative_controller``, ``nrpn_relative_controller``,
       ``per_note_management``, ``control_change``, ``pitch_bend``,
       ``per_note_pitch_bend``
   * - ``data128``
     - ``sysex8_in_1``, ``sysex8_start``, ``sysex8_continue``, ``sysex8_end``,
       ``mds_header``, ``mds_payload``
   * - ``stream``
     - ``endpoint_discovery``, ``endpoint_info_notification``,
       ``device_identity_notification``, ``endpoint_name_notification``,
       ``product_instance_id_notification``, ``jr_configuration_request``,
       ``jr_configuration_notification``, ``function_block_discovery``,
       ``function_block_info_notification``, ``function_block_name_notification``,
       ``start_of_clip``, ``end_of_clip``
   * - ``flex_data``
     - ``set_tempo``, ``set_time_signature``, ``set_metronome``,
       ``set_key_signature``, ``set_chord_name``, ``text``

The message's class has the same name as its handler, except for ``seq_start``,
``seq_continue``, and ``seq_stop`` (``system::sequence_start``,
``system::sequence_continue``, and ``system::sequence_stop``), and ``text``
(``flex_data::text_common``).

Backend classes
---------------

For each of the eight backends, ``Group``, there are four class templates, each with a
``Context`` parameter that is the type of the context:

``Group_null<Context>``
   Every handler is a ``static constexpr`` function that does nothing. Declare a member
   of this type ``[[no_unique_address]]`` to ignore a whole message type at no cost.

``Group_pure<Context>``
   An abstract class: every handler is a pure virtual function. Derive from it when you
   must handle every message, so that the compiler tells you if you miss one.

``Group_base<Context>``
   Derived from ``Group_pure<Context>``. Every handler is a virtual function that does
   nothing. Derive from it and override only the handlers you need.

``Group_function<Context>``
   Stores a ``std::function`` for each handler. ``on_handler(f)`` sets the function for
   ``handler``, and returns ``*this``, so that calls can be chained; for example,
   ``on_note_on(f)``. A message whose function is empty is ignored. Each class has a
   member type ``handler_fn`` for each handler, such as ``m2cvm_function<C>::note_on_fn``,
   which is ``std::function<void(C&, m2cvm::note_on const&)>``.

The classes for the Flex Data backend are named ``flex_data_null`` and so on, although
the configuration member is ``flex``. The handlers of the ``_pure`` and ``_base`` classes
take the context as ``Context&``. To use them through a reference to a base, see
:doc:`../howto/choose-handlers-at-run-time`.

``function_config``
-------------------

``template <typename Context> struct function_config``

   A configuration whose backends are the eight ``_function`` classes: members
   ``utility``, ``system``, ``m1cvm``, ``data64``, ``m2cvm``, ``data128``, ``stream``, and
   ``flex``. Its ``context`` member is of type ``Context`` and declared
   ``[[no_unique_address]]``.

``explicit function_config(Context c)``
   Moves ``c`` into ``context``.

``template <typename... Args> constexpr explicit function_config(std::in_place_t, Args&&... args)``
   Constructs ``context`` from ``std::forward<Args>(args)...``.

``make_ump_function_dispatcher(Context&& context = Context{})``
   Returns ``ump_dispatcher{function_config{std::forward<Context>(context)}}``. Because
   ``function_config``'s constructor takes ``Context`` by value, ``context`` must be
   copyable if it is an lvalue. To share an object that can't be copied, pass a pointer
   to it, or construct the dispatcher with ``std::in_place`` instead.

``ump_message_size``
--------------------

``constexpr unsigned ump_message_size(message_type mt)``
   Returns the number of 32-bit words in a message of type ``mt``, including the
   reserved types.
