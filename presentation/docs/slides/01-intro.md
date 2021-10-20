
- The problem - what are we solving?
  - No external tools
  - Correctness
  - Portability
  - Making constexpr libraries

- Lookup Tables
  - Approximating Functions
  - returning from a `constexpr` function
  - CRC example

- Configuration fuses
  - Between power on and `main()`
  - The traditional way
  - Using a lambda to build and return data
  - Transforming data
  - `constinit` and the linker script
  - no need for vendor specific extensions
  - Incorporate Validation

- Writing Complex Compile-Time Code
  - Examples have result types (size) known to compiler
  - Allocating Memory
  - parameters are not constant in the constexpr context
  - Turning functions in to libraries
    - Dealing with user supplied input as lambdas
  - STL containers and algorithms
    - vector
    - list
    - unordered_map
  - Building Types from user data

- String Compression

- USB Descriptors

