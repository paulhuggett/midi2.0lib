# MIDI 2.0 Lib

A MIDI 2.0 Library

This is a general purpose Library which provides the building blocks, processing and translations needed for most MIDI 2.0 devices and applications. This library aims to work on everything from embedded devices through to large scale applications with minimal code and data footprint.

This code is based on Andrew Mee’s library at <https://github.com/midi2-dev/AM_MIDI2.0Lib>. It has been heavily modified with a number of goals:

- Using C++23 features
- The library does not throw exceptions or allocate dynamic memory
- Limiting use of magic numbers and bitwise operators to define data layout and instead using the C++ type system wherever possible
- UMP conversion classes are now built on the library's `ump_dispatcher` class rather than duplicating code to extract UMP messages
- Using templates to pass callables to enable cross-callback optimization
- Significantly increasing the depth of testing

[![CI Build & Test](https://github.com/paulhuggett/midi2.0lib/actions/workflows/ci.yaml/badge.svg)](https://github.com/paulhuggett/midi2.0lib/actions/workflows/ci.yaml)
[![codecov](https://codecov.io/gh/paulhuggett/midi2.0lib/graph/badge.svg?token=8q2aEvPTyv)](https://codecov.io/gh/paulhuggett/midi2.0lib)
[![Quality gate status](https://sonarcloud.io/api/project_badges/measure?project=paulhuggett_AM_MIDI2.0Lib&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=paulhuggett_AM_MIDI2.0Lib)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/425f68679a124a1cbb0efa50342d8e8a)](https://app.codacy.com/gh/paulhuggett/midi2.0lib/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Fuzz Test](https://github.com/paulhuggett/midi2.0lib/actions/workflows/fuzztest.yaml/badge.svg)](https://github.com/paulhuggett/midi2.0lib/actions/workflows/fuzztest.yaml)
[![REUSE status](https://api.reuse.software/badge/github.com/paulhuggett/midi2.0lib)](https://api.reuse.software/info/github.com/paulhuggett/midi2.0lib)

## What does this do?

Please read the MIDI 2.0 specification on https://midi.org/specifications to understand the following.

This library can:

- Convert a MIDI 1.0 byte-stream to UMP and back
- Process and construct UMP streams
- Process and construct MIDI CI messages

This library is designed to use a small footprint. It does this by processing each UMP packet (or MIDI 1.0 Byte stream) one at a time. This way large data is handled in small chunks to keep memory small.

Note it is up to the application to:

- Store the details of remote MIDI-CI devices
- Interpret the data carried by MIDI-CI messages (for example, Profile ID bytes or Note On articulation)
- Decide when to send a NAK, and handle the NAKs that it receives

This means the overheads for a simple MIDI 2.0 device is down to a compiled size of around 10k (possibly less?), with a memory footprint of around 1k.

## Using the library

The library needs a C++23 compiler and [CMake](https://cmake.org). Add it to a CMake project with `add_subdirectory()`
or `FetchContent`, then link your target with the `midi2` library:

```cmake
include(FetchContent)
FetchContent_Declare(midi2 GIT_REPOSITORY https://github.com/paulhuggett/midi2.0lib.git GIT_TAG main)
FetchContent_MakeAvailable(midi2)
target_link_libraries(my_program PRIVATE midi2)
```

CMake fetches the library's one dependency, [icubaby](https://github.com/paulhuggett/icubaby), automatically. The
library's unit tests are built by default; set the `MIDI2_UNITTEST` option to `No` before `FetchContent_MakeAvailable()`
to skip them. The `MIDI2_DEMO` option builds the demo programs, and `MIDI2_DOC_EXAMPLES` builds (and tests) the
example programs used by the documentation.

The [docs](docs) directory holds the library's user documentation: an introduction, a tutorial, how-to guides, and a
reference for each of its parts. To build it, run Doxygen to generate XML in `docs/xml`, then run Sphinx (see
[docs/requirements.txt](docs/requirements.txt)). On Read the Docs, [docs/conf.py](docs/conf.py) runs Doxygen itself.

## Examples

Each of these examples is a complete program. There are more in the [demo](demo) directory.

### Example: Creating and Sending UMP Messages

```cpp
#include <cstdint>
#include <system_error>
// The midi2 library umbrella include file
#include <midi2/midi2.hpp>

std::error_code send_note_on(std::uint8_t channel, std::uint8_t note, std::uint16_t velocity) {
  // Create an instance of the type that represents the UMP message to be sent. Set
  // the fields of the message as desired.
  auto const message = midi2::ump::m2cvm::note_on{}
    .group(0)
    .channel(channel)
    .note(note)
    .velocity(velocity);

  // Invoke a function for each of the words that make up the complete message. There's
  // no need for this code to understand the layout or size of the message. Processing
  // stops if the function returns an error.
  return midi2::ump::apply(message, [](std::uint32_t const word) {
    // ... transmit the 32 bit word ...
    return std::error_code{};
  });
}
```

### Example: Processing a UMP stream

```cpp
#include <cstdint>
#include <iostream>
// The midi2 library umbrella include file
#include <midi2/midi2.hpp>

int main() {
  // In this scenario, we need to pass a “context” to the dispatcher,
  // which will be forwarded to the callbacks as they are invoked.
  // The context facilitates efficient state sharing among message
  // handlers, but this is not necessary for our simple example.
  // Therefore, a struct with no members will suffice.
  struct context {};

  // Create the dispatcher with a default-initialized context.
  auto dispatcher = midi2::ump::make_ump_function_dispatcher(context{});

  // Ask the dispatcher for its configuration object and install handlers for MIDI2
  // note on/off channel voice messages.
  dispatcher.config()
    .m2cvm
      .on_note_off([](context&, midi2::ump::m2cvm::note_off const& noff) {
        std::cout << "note off: #" << unsigned{noff.note()}
                  << ", velocity " << noff.velocity() << '\n';
      })
      .on_note_on([](context&, midi2::ump::m2cvm::note_on const& non) {
        std::cout << "note on: #" << unsigned{non.note()}
                  << ", velocity " << non.velocity() << '\n';
      });

  // Send note-on/off messages to the dispatcher.
  for (std::uint32_t const v : {0x40913C00U, 0x7F100000U, 0x40813C00U, 0x7FFF0000U}) {
    dispatcher.dispatch(v);
  }
}
```

The output is:

```
note on: #60, velocity 32528
note off: #60, velocity 32767
```

### Example: Translate MIDI 1.0 Byte stream to UMP

```cpp
#include <cstdint>
#include <iostream>
// The midi2 library umbrella include file
#include <midi2/midi2.hpp>

using namespace midi2::literals;  // for the _b (std::byte) literal

int main() {
  // Convert the bytestream to UMP group 0 and write it to stdout.
  constexpr auto group = std::uint8_t{0};
  midi2::bytestream::to_ump bs2ump{group};

  // A bytestream containing MIDI1 note-on events with running status.
  for (auto const b : {0x81_b, 0x60_b, 0x50_b, 0x70_b, 0x70_b}) {
    // Push each byte into the bs2ump instance.
    bs2ump.push(b);

    // Pull as many 32-bit UMP values as are available and display them.
    while (!bs2ump.empty()) {
      std::cout << std::hex << bs2ump.pop() << '\n';
    }
  }
}
```

### Example: Create MIDI-CI Messages

```cpp
#include <cstddef>
#include <format>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>
// The midi2 library umbrella include file
#include <midi2/midi2.hpp>

using namespace midi2::literals;  // for the _b7, _b14 and _b28 literals

namespace {

std::vector<std::byte> discovery() {
  constexpr auto my_muid = midi2::ci::muid{0x01234567U};  // Use a proper random number!
  constexpr midi2::ci::header header{
      .device_id = 0x7F_b7, .version = 2_b7, .remote_muid = my_muid, .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery discovery{.manufacturer = {0x12_b7, 0x23_b7, 0x34_b7},
                                           .family = 0x1779_b14,
                                           .model = 0x2B5D_b14,
                                           .version = {0x4E_b7, 0x3C_b7, 0x2A_b7, 0x18_b7},
                                           .capability = 0x7F_b7,
                                           .max_sysex_size = 256_b28,
                                           .output_path_id = 0x71_b7};
  std::vector<std::byte> message;
  // A back_insert_iterator can't run out of space, so use trivial_sentinel as the end of the output range.
  midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{}, header, discovery);
  return message;
}

}  // end anonymous namespace

int main() {
  for (auto const b : discovery()) {
    std::cout << std::format("{:02X} ", std::to_underlying(b));
  }
  std::cout << '\n';
}
```

### Example: Process MIDI-CI Messages

MIDI-CI requires a lot of SysEx messages. This library abstracts the complexity of building and parsing MIDI-CI Messages.

```cpp
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
// The midi2 library umbrella include file
#include <midi2/midi2.hpp>
// The umbrella file does not include the MIDI-CI dispatcher
#include <midi2/ci/ci_dispatcher.hpp>

using namespace midi2::literals;  // for the _b and _b7 literals

namespace {

// We must pass a “context” to the dispatcher, which will be forwarded to
// each of the dispatcher's callbacks. The context lets message handlers share
// state but we don’t need that here, so a struct with no members will suffice.
struct context {};

// The size of the dispatcher's internal buffer.
constexpr auto buffer_size = std::size_t{256};

auto setup_ci_dispatcher(midi2::ci::b7 const device_id, std::uint8_t const group, midi2::ci::muid const my_muid) {
  // Create a CI dispatcher instance using std::function<> for
  // all of its handler functions.
  auto dispatcher = midi2::ci::make_function_dispatcher<context, buffer_size>(device_id, group);
  auto &config = dispatcher.config();

  // Register a handler for checking whether a message is addressed
  // to this receiver. The default handler rejects all messages.
  config.system.on_check_muid(
      [my_muid](context &, std::uint8_t /*group*/, midi2::ci::muid const m) { return m == my_muid; });

  // Register a handler for Discovery messages.
  config.management.on_discovery([](context &, midi2::ci::header const &h, midi2::ci::discovery const &d) {
    std::cout << std::format("Discovery from MUID 0x{:X}: family=0x{:X}, model=0x{:X}, max-sysex-size={}\n",
                             h.remote_muid, d.family, d.model, d.max_sysex_size);
    // Send a reply to this message...
  });
  return dispatcher;
}

}  // end anonymous namespace

int main() {
  constexpr auto my_muid = midi2::ci::muid{0x01234567U};  // Use a proper random number!
  constexpr auto my_group = std::uint8_t{0};
  constexpr auto device_id = 0_b7;
  auto dispatcher = setup_ci_dispatcher(device_id, my_group, my_muid);

  // A system exclusive message containing a CI discovery request.
  constexpr std::array message{0x7E_b, 0x7F_b, 0x0D_b, 0x70_b, 0x02_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x7F_b,
                               0x7F_b, 0x7F_b, 0x7F_b, 0x12_b, 0x23_b, 0x34_b, 0x79_b, 0x2E_b, 0x5D_b, 0x56_b,
                               0x01_b, 0x00_b, 0x00_b, 0x00_b, 0x7F_b, 0x00_b, 0x02_b, 0x00_b, 0x00_b, 0x00_b};
  for (auto const b : message) {
    dispatcher.dispatch(b);
  }
}
```

---
