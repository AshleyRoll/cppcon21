## Config Fuses

- Initialise hardware before the processor starts
- Fixed locations in Flash memory, filled with bit-mapped magic values
- Sets up
  - clocks, memory segments, watchdog timer
  - JTAG debug, code security, and much more



<!-- down -->
## Config Fuses

- Vendor specific compiler extensions to set fuses
  - pragmas, or similar
  - special macros
- Rely on C style `#define` for bit values
- No validation, potentially very complex



<!-- down -->
### Lets do better

- Strongly type all configuration registers
- Provide a "builder" object
- Render a constinit object, at compile-time
- Place it in Flash using segments and linker script

