# MIDI2 Library

## Overview

- Translators
  - To/From Bytestream
    - midi2::bytestream::to_ump
    - midi2::bytestream::to_bytestream
    - midi2::bytestream::usbm1_to_bytestream

  - UMP MIDI 1 <-> MIDI 2 conversion
    - midi2::ump::ump_to_midi2
    - midi2::ump::to_midi1

- Dispatchers
  - midi2::ci::ci_dispatcher
  - midi2::ump::ump_dispatcher

- Message Construction
  - midi2::ci::create_message()
  - Types in the midi2::ump namespace (e.g. midi2::ump::m2cvm::note_on)
    in conjunction with midi2::ump::apply()

- MIDI-specific Codecs
  - Mcoded7 (midi2::mcoded7::encoder and midi2::mcoded7::decoder)
  - The 7-bit text used by MIDI-CI Property Exchange (specializations of midi2::transcoder in ci7text.hpp)

## Using Translators

Translators are used to convert between different MIDI message formats. They
share a common, very simple, interface, which makes it easy to switch between
different types of translation.

The library provides five different translators. They are:

- To and from MIDI 1.0 byte-streams
  - MIDI 1.0 bytestream to UMP (\ref midi2::bytestream::to_ump)
  - UMP MIDI 1.0 to MIDI 1.0 bytestream (\ref midi2::bytestream::to_bytestream)
  - USB-MIDI Event Packets to MIDI 1.0 bytestream (\ref midi2::bytestream::usbm1_to_bytestream)

- UMP MIDI 1 <-> MIDI 2 conversion
  - MIDI 1.0 UMP packets to MIDI 2.0 UMP (\ref midi2::ump::ump_to_midi2)
  - MIDI 2.0 UMP packets to MIDI 1.0 UMP (\ref midi2::ump::to_midi1)

The translators provide a common interface with `push()`, `empty()`,
`pop()`, and `reset()` methods. Some have additional member functions to
control filtering of input messages. The `input_type` and `output_type`
typedefs can be used to determine the correct types for the input and output
messages. This is enforced by the \ref midi2::translator concept.

```cpp
class translator {
public:
  using input_type = ...;   // Type of input messages
  using output_type = ...;  // Type of output messages

  void push(input_type in); // Push an input message into the translator
  bool empty() const;       // Is output available?
  output_type pop();        // Pop an output message from the translator
  void reset();             // Reset the translator state
};
```

The code snippet below shows the general usage pattern for the translators.
They are designed to handle streaming data: you create a translator instance,
push each input message byte or 32-bit value into it and then immediately pull
any translated output from the translator's internal FIFO. The resulting output
can then be dispatched to its next destination.

```cpp
translator t;
for (translator::input_type in : input) {
  t.push(in);
  while (!t.empty()) {
    translator::output_type out = t.pop();
    // Send 'out' to its next destination
  }
}
```

(Replace the “translator” placeholder in the code above with one of the real
translator types.)

## Using Dispatchers

The library provides two different dispatchers. The dispatchers accept input
message bytes or 32-bit values through their `dispatch()` member function,
decode them, and call functions provided by the application to handle those
messages. They are for:

- MIDI Capability Inquiry message (\ref midi2::ci::ci_dispatcher)
- Universal MIDI Packet (UMP) messages (\ref midi2::ump::ump_dispatcher)

When instantiating one of the dispatchers, you must provide a configuration
type. This type defines:

1. The collection of functions that will handle the dispatched messages
2. A “context” object that will be passed to each of the message handling
   functions. See below for more information.
3. (CI dispatcher only) A size for the dispatcher’s internal buffer.

The full requirements for the configuration type are described by the
`midi2::ci::ci_dispatcher_config` or `midi2::ump::ump_dispatcher_config`
concepts.

### Contexts

A “context” object is passed to each of the message handling functions to
enable them to efficiently share state. This can be a pointer to an object
in the larger system or an instance owned by the configuration object itself.
If not needed by the handlers, it can be an empty struct declared with
`[[no_unique_address]]` so that it takes no space.

The context is passed by reference, so its type need not be copyable and a
change made to it by one handler is seen by those called later. A context which
can be neither copied nor moved can be constructed in place by passing
`std::in_place`, followed by the arguments for its constructor, to the
dispatcher's constructor.

### Predefined message handlers

The handlers for each group of messages (for example, the MIDI 2.0 channel
voice messages) are provided by a separate class. The predefined classes are in
the `midi2::ump::dispatcher_backend` and `midi2::ci::dispatcher_backend`
namespaces. There are four variations of each, whose names end in `_null`,
`_pure`, `_base`, and `_function` (for example, `m2cvm_null`, `m2cvm_pure`,
`m2cvm_base`, and `m2cvm_function`).

1. Null

   A group of message handlers in which every function is a static constexpr
   function that does nothing. They can be very useful to simply drop an entire
   class of input messages. Since they are constexpr, the compiler can often
   remove swathes of code leading up to the call of these functions so they can
   be very efficient.

2. Pure Virtual

   The message handlers are defined as pure virtual. The library’s unit testing
   mocks make extensive use of these classes to track and verify the delivered
   messages.

3. Virtual Base

   Each message handler class derives from the matching pure-virtual class. It
   provides concrete implementations that each do nothing. You can derive from
   one of these classes and override one or more of the handlers with very
   little code. The disadvantage is obviously that each message incurs the
   overhead of a virtual method call.

4. std::function<>

   The easiest configuration type to use, but may not always be the most time
   or space efficient. Use judiciously! The simplest way to use the
   `std::function<>`-based handlers is to create the dispatcher with one of the
   helper functions, `midi2::ump::make_ump_function_dispatcher()` or
   `midi2::ci::make_function_dispatcher()`, and then install a handler for each
   message of interest through the dispatcher's `config()` member function.
   Messages without a handler are ignored. The exception is the CI
   dispatcher's `check_muid` handler: without one, every message that is not
   broadcast is rejected, so install one with `config().system.on_check_muid()`. Note that
   `std::function<>` may allocate memory or throw exceptions.

5. Custom message handlers

   Any type that provides the required member functions can be used. The
   `midi2::ump::dispatcher_backend` and `midi2::ci::dispatcher_backend`
   namespaces contain a concept for each group of messages (for example,
   `midi2::ump::dispatcher_backend::m2cvm`) that describes what is needed. A
   custom configuration can mix custom handlers with the predefined ones. The
   configuration below counts MIDI 2.0 note-on messages and ignores everything
   else:

   ```cpp
   struct counter {
     unsigned notes = 0;
   };
   namespace backend = midi2::ump::dispatcher_backend;
   struct note_counter : backend::m2cvm_null<counter> {
     static void note_on(counter& c, midi2::ump::m2cvm::note_on const&) { ++c.notes; }
   };
   struct config {
     counter context;
     [[no_unique_address]] backend::utility_null<counter> utility;
     [[no_unique_address]] backend::system_null<counter> system;
     [[no_unique_address]] backend::m1cvm_null<counter> m1cvm;
     [[no_unique_address]] backend::data64_null<counter> data64;
     [[no_unique_address]] note_counter m2cvm;
     [[no_unique_address]] backend::data128_null<counter> data128;
     [[no_unique_address]] backend::stream_null<counter> stream;
     [[no_unique_address]] backend::flex_data_null<counter> flex;
   };

   midi2::ump::ump_dispatcher<config> dispatcher{config{}};
   ```

## Message Construction

### MIDI CI

The MIDI-CI messages are described by types in the midi2::ci namespace (for
example, midi2::ci::discovery). midi2::ci::create_message() writes a MIDI-CI
message to an output range, given a midi2::ci::header and one of these types.
It returns an iterator past the last byte written. The bytes do not include the
F0 and F7 bytes that start and end a System Exclusive message.

If the message does not fit, the iterator returned is equal to the end of the
range, and part of the message may have been written. A message that fills the
range exactly also returns the end of the range. To tell the two apart, use a
range at least one byte larger than the largest message you will write:

```cpp
std::array<std::byte, max_size + 1> buffer{};
auto const pos = midi2::ci::create_message(buffer.begin(), buffer.end(), header, discovery);
if (pos == buffer.end()) {
  // The message is larger than max_size bytes
}
```

When the output
iterator cannot run out of space (for example, a `std::back_insert_iterator`),
pass midi2::ci::trivial_sentinel as the end of the output range:

```cpp
std::vector<std::byte> message;
midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{}, header, discovery);
```

### UMP Messages

Creating a UMP message is a simple matter of instantiating the relevant type. The midi2::ump namespace contains a type
that corresponds to each of the possible UMP messages. These are further grouped in namespaces according to the message
category. These categories are:

| Message Type           | Namespace               |
| ---------------------- | ----------------------- |
| Utility                | `midi2::ump::utility`   |
| System Common          | `midi2::ump::system`    |
| System Real Time       | `midi2::ump::system`    |
| MIDI 1.0 Channel Voice | `midi2::ump::m1cvm`     |
| Data 64-Bit            | `midi2::ump::data64`    |
| MIDI 2.0 Channel Voice | `midi2::ump::m2cvm`     |
| Data 128-Bit           | `midi2::ump::data128`   |
| UMP Stream             | `midi2::ump::stream`    |
| Flex Data              | `midi2::ump::flex_data` |

Each type can be default-constructed. This will set all fields, except those that identify the message type, to 0.
Accessor functions, whose names generally match those of the field in question, allow the individual fields of the
message to be read or written. "Getter" accessors take no arguments and return the associated value; "setter"
accessors take the value to be set and return `*this`, enabling calls to be chained. Note that there is no "setter"
method for the fields that identify the message: these are set implicitly when the message instance is created and are
read-only.

For example, to create a MIDI 1.0 Note-on message and set its various fields:

```cpp
auto const non = midi2::ump::m1cvm::note_on{}.group(g).channel(c).note(n).velocity(v);
```

This approach means that calling code does not need to know which of the message's constituent words contains each
field: that is the responsibility of the message type itself. Having said that, the UMP message types are all
"tuple-like". This means that, when necessary, the individual words of the message can be accessed as members of the
tuple:

```cpp
auto const noff = midi2::ump::m2cvm::note_off{}
  .group(g)
  .channel(c)
  .note(n)
  .velocity(v)
  .attribute(a);

// You can use std::tuple_size<> to determine the number of words in the message (2).
constexpr auto words = std::tuple_size_v<midi2::ump::m2cvm::note_off>;

// Extract the first word of the message. A word can be converted to std::uint32_t.
auto const w0 = static_cast<std::uint32_t>(get<0>(noff));

// We can also use a structured binding declaration.
auto const [x0, x1] = noff;
```

#### Fields that hold arrays

Some messages carry a sequence of bytes:

- The System Exclusive messages: midi2::ump::data64::sysex7_in_1 and the other three data64 sysex7 messages, and
  midi2::ump::data128::sysex8_in_1 and the other three data128 sysex8 messages (`data()`)
- midi2::ump::flex_data::text_common (`data()`)
- midi2::ump::stream::endpoint_name_notification and midi2::ump::stream::function_block_name_notification (`name()`)
- midi2::ump::stream::product_instance_id_notification (`pid()`)

Each of these can be treated as a container of bytes. It has `size()`, `empty()`, `begin()`, and `end()`, and
`operator[]` reads or writes a single byte. The named accessor (`data()`, `name()`, or `pid()`) reads or writes a single
byte given its index. It also sets all of the bytes from an initializer list, a range (with an optional projection), or
an iterator and sentinel pair. For a System Exclusive message, the number of bytes in the message is set to the number
copied. For text, a name, or a product instance ID, the bytes that were not set are filled with zeros.

```cpp
auto sx = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x7E, 0x7F, 0x06, 0x01});
sx[3] = 0x02;
for (auto const b : sx) {
  // ... 0x7E, 0x7F, 0x06, 0x02
}

using namespace std::string_view_literals;
auto const en = midi2::ump::stream::endpoint_name_notification{}.name(u8"My Device"sv);
// en.size() is 9
```

midi2::ump::stream::device_identity_notification has two arrays of bytes, so it cannot be treated as a single container.
Instead, `dev_manuf_sysex_id()` and `sw_revision()` each read or write one byte given its index, return all of the bytes
as a `std::array`, and set the bytes from an initializer list, a range, or an iterator and sentinel pair. Any bytes that
are not set are filled with zeros:

```cpp
auto const di = midi2::ump::stream::device_identity_notification{}
  .dev_manuf_sysex_id({0x7D})  // A one-byte manufacturer ID is stored as 7D 00 00
  .sw_revision({1, 2, 3, 4});
```

#### Apply

midi2::ump::apply() calls a function for each of the 32-bit words of a UMP message in turn, starting with the first.
The calling code doesn't need to know how many words the message has. If the function returns a value that converts to
`true` (such as a `std::error_code` holding an error), no more words are passed to it and that value is returned:

```cpp
auto const message = midi2::ump::m2cvm::note_on{}.group(0).channel(1).note(60).velocity(0x8000);
if (std::error_code const err = midi2::ump::apply(message, [](std::uint32_t const word) {
      return transmit(word);  // Returns a std::error_code
    })) {
  // Handle the error
}
```

midi2::ump::check() checks that each of the words of a UMP message is consistent.

\example send_ump.cpp

This example demonstrates how to create UMP messages, fill in their fields, and transmit the resulting sequence of
32-bit values. It sends a MIDI 1.0 program change message, then MIDI 2.0 note-on and note-off messages for three notes;
the same principle applies to all UMP message types. In this example the 32-bit values are simply written to the
console, but a real implementation would likely transmit them using a communications protocol such as USB-MIDI.

Expected output:

```
Program change: 0x20c12a00
Note on: 0x40913c00 0x27100000
Note on: 0x40914000 0x27100000
Note on: 0x40914300 0x27100000
Note off: 0x40813c00 0x27100000
Note off: 0x40814000 0x27100000
Note off: 0x40814300 0x27100000
```

\example ump_dispatcher_function.cpp

This demo program shows the UMP dispatcher being used with the provided "function" backend class.

The dispatcher's backend class determines how each UMP message is handled. The "function" backend uses
std::function<> to make it simple to use callback functions (be they lambdas or other types of callable object) as
message handlers.

Be aware that std::function<> is powerful and simple to use but may allocate memory or throw exceptions.
For resource-constrained applications, one of the other backend options may be more suitable.

Expected output:

```
note on: #60, velocity 32528
note off: #60, velocity 32767
```

\example midi1_to_ump.cpp

This program uses midi2::bytestream::to_ump to translate a MIDI 1.0 bytestream, which contains two note-on messages
using running status, to UMP.

Expected output:

```
Bytestream input: 0x81 0x60 0x50 0x70 0x70
UMP Packets: 0x20816050 0x20817070
```

\example ci_dispatcher.cpp

This program passes the bytes of a MIDI-CI Discovery message to a midi2::ci::ci_dispatcher that uses the
`std::function<>` backend. Its Discovery handler prints the message's header and body.

Expected output:

```
device-id=0x0, version=0x2, remote-MUID=0x0, local-MUID=0xFFFFFFF
manufacturer=[0x12,0x23,0x34], family=0x1779, model=0x2B5D, version=[0x1,0x0,0x0,0x0], capability=0x7F, max-sysex-size=0x100, output-path-id=0x0
```

\example ci_create_discovery.cpp

This program uses midi2::ci::create_message() to build a MIDI-CI Reply to Discovery message in an array, then prints
its bytes.

Expected output:

```
7E 00 0D 71 02 67 0A 0D 09 7F 7F 7F 7F 12 23 34 79 2E 5D 56 01 00 00 00 7F 00 02 00 00 00 00
```

\example pe_viewer.cpp

This program displays the contents of a MIDI-CI Property Exchange message carried by an array of UMP System Exclusive
(7-bit) packets. The packets are passed to a ci_dispatcher whose configuration streams Property Exchange messages: the
header and then the property data are given to a single [peejay](https://github.com/paulhuggett/peejay) JSON parser
piece by piece, so the message can be much larger than the dispatcher's 32-byte buffer. A peejay::writer backend
prints the JSON with indentation. The dispatcher and parser are reset for each message, so that any number may be sent.
The output begins:

```
Header:
{
  "status": 200
}
Property data:
{
  "manufacturerId": [
    125,
    0,
    0
```

\example pe_zlib_viewer.cpp

This program is a companion to pe_viewer.cpp. Its Property Exchange message carries a ProgramList whose JSON property
data has been compressed with [zlib](https://zlib.net) and then encoded using Mcoded7, as described in "Compressed and
Encoded JSON Property Data" in the Common Rules for MIDI-CI Property Exchange specification. The header's
`mutualEncoding` property is `"zlib+Mcoded7"`. Each piece of property data from the ci_dispatcher is decoded by
midi2::mcoded7::decoder and inflated by zlib before it is passed to the JSON parser, so neither the encoded nor the
decompressed data is ever held in full. The 654 bytes of JSON travel as 256 bytes of Mcoded7. Property data whose
`mutualEncoding` is `"Mcoded7"` (encoded but not compressed) or `"ASCII"` (neither) is also supported.

As in pe_viewer.cpp, a peejay::writer prints the JSON; it is extended to remember the header's `mutualEncoding` and
`mediaType` values. Any other `mutualEncoding`, a `mediaType` other than `application/json`, and JSON which is not
well-formed UTF-8 are reported as errors.

The output begins:

```
Header:
{
  "status": 200,
  "mutualEncoding": "zlib+Mcoded7"
}
Property data:
[
  {
    "title": "Grand Piano",
    "bankPC": [
      0,
      0,
      0
    ]
  },
```

\example workbench_device.cpp

This macOS program is a MIDI 2.0 device that the [MIDI 2.0 Workbench](https://github.com/midi2-dev/MIDI2.0Workbench)
can connect to and test. It creates a CoreMIDI virtual source and destination with the same name, both using the MIDI
2.0 protocol, which the Workbench lists as a UMP endpoint. A ump_dispatcher answers the UMP Stream messages (Endpoint
Discovery, Stream Configuration Request, and Function Block Discovery). The device's single function block carries
MIDI-CI on group 1 (group 0 in the code): SysEx7 packets on that group are passed to a ci_dispatcher, and the replies
are built with ci::create_message() and sent as SysEx7. The device answers Discovery, Inquiry: Endpoint, Profile
Inquiry, Property Exchange Capabilities, and Property Exchange Get for the `ResourceList` and `DeviceInfo` resources.
A Get request's header is parsed into a DOM by [peejay](https://github.com/paulhuggett/peejay), and the `resource`
member is read from it; the header's other members are ignored. A header which is not valid JSON gets a reply with
status 400.
It also handles Invalidate MUID: when its own MUID is invalidated, or another device's Discovery uses the same MUID, it
picks a new MUID and broadcasts a Discovery message so that other devices can connect to it again. It invalidates its
MUID when it quits. Channel voice note messages are logged.

Run the program, then start (or refresh) the Workbench. An optional argument changes the endpoint name. Press Ctrl-C to
quit. Usage: `demo_workbench_device [-debug] [endpoint-name]`. Each message the device handles is logged, for example:

```
"midi2lib Demo Device" is ready (MUID 0x49BD0D0). Press Ctrl-C to quit.
Endpoint Discovery: UMP v1.2 filter=0x1F
Function Block Discovery: block=0xFF filter=0x03
MIDI-CI Discovery: from MUID 0x317967A, CI version 0x02
```

Pass `-debug` to also log each UMP message received (`<-`) or sent (`->`) as hexadecimal words, and the bytes of each
MIDI-CI System Exclusive message:

```
<- UMP: 30167E7F 0D720256
<- UMP: 30266848 003B0A7D
<- UMP: 30327B00 00000000
<- SysEx (14 bytes): 7E 7F 0D 72 02 56 68 48 00 3B 0A 7D 7B 00
MIDI-CI Inquiry: Endpoint: status=0x00
-> SysEx (29 bytes): 7E 7F 0D 73 02 3B 0A 7D 7B 56 68 48 00 00 0D 00
   6D 69 64 69 32 6C 69 62 2D 30 30 30 31
-> UMP: 30167E7F 0D73023B
...
```
