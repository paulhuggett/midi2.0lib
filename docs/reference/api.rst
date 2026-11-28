API listing
===========

This page is generated from the comments in midi2's headers. The other reference pages
describe how the pieces work together; this page lists the main public declarations.
The UMP and MIDI-CI message types follow the patterns described in :doc:`ump-messages`
and :doc:`ci-messages`, and are not listed individually.

Shared concepts
---------------

.. doxygenconcept:: midi2::translator

.. doxygenconcept:: midi2::dispatcher

UMP messages
------------

.. doxygenenum:: midi2::ump::message_type

.. doxygenconcept:: midi2::ump::ump_message

.. doxygenfunction:: midi2::ump::apply

.. doxygenfunction:: midi2::ump::check

.. doxygenfunction:: midi2::ump::mcm_scale

.. doxygenenum:: midi2::ump::control

UMP dispatcher
--------------

.. doxygenclass:: midi2::ump::ump_dispatcher
   :members:

.. doxygenconcept:: midi2::ump::ump_dispatcher_config

.. doxygenstruct:: midi2::ump::function_config
   :members:

.. doxygenfunction:: midi2::ump::make_ump_function_dispatcher

.. doxygenfunction:: midi2::ump::ump_message_size

UMP backends
^^^^^^^^^^^^

The classes for MIDI 2.0 Channel Voice messages are shown. The classes for the other
message types have the same form.

.. doxygenconcept:: midi2::ump::dispatcher_backend::m2cvm

.. doxygenstruct:: midi2::ump::dispatcher_backend::m2cvm_null
   :members:

.. doxygenstruct:: midi2::ump::dispatcher_backend::m2cvm_pure
   :members:

.. doxygenstruct:: midi2::ump::dispatcher_backend::m2cvm_base
   :members:

.. doxygenclass:: midi2::ump::dispatcher_backend::m2cvm_function
   :members:

Protocol translators
--------------------

.. doxygenclass:: midi2::ump::ump_to_midi2
   :members:

.. doxygenclass:: midi2::ump::to_midi1
   :members:

Bytestream translators
----------------------

.. doxygenclass:: midi2::bytestream::to_ump
   :members:

.. doxygenclass:: midi2::bytestream::to_bytestream
   :members:

.. doxygenclass:: midi2::bytestream::usbm1_to_bytestream
   :members:

.. doxygenenum:: midi2::bytestream::status

.. doxygenfunction:: midi2::bytestream::is_system_real_time_message

.. doxygenfunction:: midi2::bytestream::is_status_byte

MIDI-CI messages
----------------

.. doxygentypedef:: midi2::ci::b7

.. doxygentypedef:: midi2::ci::b14

.. doxygentypedef:: midi2::ci::b28

.. doxygentypedef:: midi2::ci::muid

.. doxygenvariable:: midi2::ci::broadcast_muid

.. doxygenvariable:: midi2::ci::max_user_muid

.. doxygenenum:: midi2::ci::message

.. doxygenstruct:: midi2::ci::header
   :members:

.. doxygenstruct:: midi2::ci::discovery
   :members:

.. doxygenstruct:: midi2::ci::endpoint_reply
   :members:

.. doxygenclass:: midi2::ci::property_exchange::property_exchange
   :members:

.. doxygenstruct:: midi2::ci::property_exchange::chunk_info
   :members:

.. doxygenstruct:: midi2::ci::property_exchange::stream_start
   :members:

.. doxygenstruct:: midi2::ci::property_exchange::stream_chunk
   :members:

.. doxygenconcept:: midi2::ci::ci_message

.. doxygenstruct:: midi2::ci::trivial_sentinel
   :members:

``create_message()`` has one overload for the messages whose body has a fixed size, and
one for each message with variable-length data:

.. doxygenfunction:: midi2::ci::create_message(O first, S const last, header const &hdr, T const &t)

.. doxygenfunction:: midi2::ci::create_message(O first, S const last, header const &hdr, endpoint_reply const &reply)

MIDI-CI dispatcher
------------------

.. doxygenclass:: midi2::ci::ci_dispatcher
   :members:

.. doxygenconcept:: midi2::ci::ci_dispatcher_config

.. doxygenconcept:: midi2::ci::ci_dispatcher_stream_config

.. doxygenstruct:: midi2::ci::function_config
   :members:

.. doxygenstruct:: midi2::ci::function_stream_config
   :members:

MIDI-CI backends
^^^^^^^^^^^^^^^^

The classes for Property Exchange are shown. The classes for the other groups have the
same form.

.. doxygenconcept:: midi2::ci::dispatcher_backend::property_exchange

.. doxygenconcept:: midi2::ci::dispatcher_backend::property_exchange_stream

.. doxygenstruct:: midi2::ci::dispatcher_backend::property_exchange_base
   :members:

.. doxygenclass:: midi2::ci::dispatcher_backend::property_exchange_function
   :members:

.. doxygenclass:: midi2::ci::dispatcher_backend::property_exchange_stream_function
   :members:

Codecs
------

.. doxygenclass:: midi2::mcoded7::encoder
   :members:

.. doxygenclass:: midi2::mcoded7::decoder
   :members:
