
- The problem - what are we solving?
  - No external tools
  - Correctness
  - Portabilty

- Motivating Examples
  - Look up tables
  - Configuration fuses
  - Prebuilt messages - USB descriptors
  - Compressed Data / Strings / Binary Blobs

- Intro to `constexpr`
  - What can be returned?
  - `constinit`
  - `consteval`

- Lookup Tables
  - Approximating Functions
  - returning from a `constexpr` function
  - CRC example

- Configuration fuses
  - Between power on and `main()`
  - Hardware uses data structures too
  - The traditional way
  - Using a lambda to build and return data
  - Transforming data
  - `constinit` and the linker script
  - no need for vendor specific extensions
  - Incorporate Validation

- Dicsussion
  - Examples have result types (size) known to compiler
  - Allocating Memory
  - `std` Containers
  - parameters are not constant in the constexpr context

- Writing Complex Compile-Time Code
  - STL containers and algorithms
  - Building Types from user data
  - Limitations and Work-arounds

- String Compression

- USB Descriptors

