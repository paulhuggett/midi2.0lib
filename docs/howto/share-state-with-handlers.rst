Share state with message handlers
=================================

Every handler receives a reference to the configuration's **context**, so the handlers
can share state with each other and with the rest of your program. This guide shows the
ways to provide one. They apply equally to :cpp:class:`midi2::ump::ump_dispatcher` and
:cpp:class:`midi2::ci::ci_dispatcher`.

.. literalinclude:: ../examples/howto_context.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   in place: notes=11
   by reference: notes=1

A context held by value
-----------------------

The simplest context is an ordinary value stored in the configuration, which the
dispatcher stores in turn. Read it with ``dispatcher.config().context``. The context is
passed by reference, so a change made by one handler is seen by those called later. If
the handlers don't need any state, use an empty struct with ``[[no_unique_address]]`` so
that it takes no space.

A context constructed in place
------------------------------

A context that can be neither copied nor moved, such as one that holds a
``std::atomic<>`` or a ``std::mutex``, can't be passed to the dispatcher's constructor.
Instead, pass ``std::in_place`` followed by the arguments for the context's constructor,
as the first part of the program does. The dispatcher passes them to the configuration's
``std::in_place_t`` constructor, which :cpp:struct:`midi2::ump::function_config`,
:cpp:struct:`midi2::ci::function_config`, and
:cpp:struct:`midi2::ci::function_stream_config` provide. A :cpp:class:`midi2::ci::ci_dispatcher`
can also be given a Device ID and group before ``std::in_place``.

A pointer to your own object
----------------------------

The context can be a pointer to an object elsewhere in your program. The handlers then
receive a reference to the pointer. :doc:`carry-midi-ci-over-ump` uses this to give the
handlers of a UMP dispatcher access to an object that owns a MIDI-CI dispatcher.

A configuration held by reference
---------------------------------

A dispatcher can refer to a configuration that you own instead of holding one. Pass
``std::ref(cfg)`` to the constructor: the class template argument is deduced to be
``std::reference_wrapper<config>``, and ``config()`` returns a reference to ``cfg``. The
configuration must outlive the dispatcher. The second part of the program does this.

.. note::

   Pass the context to :cpp:func:`midi2::ump::make_ump_function_dispatcher` as an
   rvalue, such as ``context{}`` or a pointer. The function doesn't accept an lvalue.
