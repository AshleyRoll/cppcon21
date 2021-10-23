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



<!-- down -->
```C++
enum class WatchDogMode {/* ... */};
enum class OscillatorMode {/* ... */};

class ConfigBuilder
{
public:
    constexpr void set_watchdog(WatchDogMode wtd)
    {
        m_Wdt = wtd;
    }

    constexpr void set_oscillator(OscillatorMode osc)
    {
        m_Osc = osc;
    }

    constexpr auto build()
    {
        //...
    }

private:
    WatchDogMode m_Wdt {WatchDogMode::Disabled};
    OscillatorMode m_Osc {OscillatorMode::InternalRC};
};
```
<!-- .element: class="r-stretch" -->



```C++
constexpr auto build()
{
    // Serialise all the registers in correct order and into
    // the correct bit locations without relying on mapping
    // structs and packing correctly
    // Lets pretend the registers are 32 bits, and there 2 of them
    std::array<std::uint32_t, 2> registers;

    auto wdt = static_cast<std::uint32_t>(m_Wdt);
    auto osc = static_cast<std::uint32_t>(m_Osc);

    // lets pretend we need values and their complement
    registers[0] = (wdt << 24u) | (~wdt & 0x0000'00FF);
    registers[1] = (osc << 24u) | (~osc & 0x0000'00FF);

    return registers;
}
```
<!-- .element: class="r-stretch" -->



<!-- down -->
### Using it

```C++
[[gnu::section(".config_registers"), gnu::used]]
constinit const auto CONFIG_REGISTERS = []{
    ConfigBuilder cfg;
    cfg.set_watchdog(WatchDogMode::Enabled_10ms);
    cfg.set_oscillator(OscillatorMode::Crystal);
    return cfg.build();
}();
```
- captureless lambda is constexpr
  <!-- .element: class="fragment" -->
- constinit const ensures it is immutable and built at compile-time
  <!-- .element: class="fragment" -->
- gnu:section places value in .config_registers section
  <!-- .element: class="fragment" -->
- gnu:unused stops compiler discarding it if not referenced
  <!-- .element: class="fragment" -->



<!-- down -->
### Linker script

- Defines layout for final binary/image
  <!-- .element: class="fragment" -->
- Maps sections into memory regions
  <!-- .element: class="fragment" -->
- Normally you never need to know..
  <!-- .element: class="fragment" -->
  - until you do..
  <!-- .element: class="fragment" -->



<!-- down -->
- We need to ensure CONFIG_REGISTERS is placed in a very
  specific location for the hardware
- Lets pretend that is address 0x0100
  <!-- .element: class="fragment" -->
- Define MEMORYs in the script, add one for config registers
  <!-- .element: class="fragment" -->
- Map our .config_registers segment to that
  <!-- .element: class="fragment" -->



```
/* World's worst linker script */

MEMORY
{
  config : ORIGIN = 0x0100, LENGTH = 0x8
  prog (rx) : ORIGIN = 0x1000, LENGTH = 0x100000
}

SECTIONS
{
  /* place config registers in to the right memory location  */
  .config_registers : {
    KEEP(*(.config_registers))
  } > config

  /* everyting else - don't do this */
  .text : { *(.text) *(.text.*) } > prog
  .data : { *(.data) } > prog
  .bss : { *(.bss) } > prog
  /DISCARD/ : { *(.*) }
}

```
<!-- .element: class="r-stretch" -->



<!-- down -->
Compile and link with the linker script

```bash
gcc -O3 --std=c++20 -c main.cpp -o main.o
ld -o config.out -T config_register.ld main.o
objdump -ds config.out
```




<!-- down -->
```text
$ ./build.sh

config.out:     file format elf64-x86-64

Contents of section .config_registers:
 0100 fd000002 fc000003                    ........
Contents of section .text:
 1000 31c0c3                               1..

Disassembly of section .text:

0000000000001000 <main>:
    1000:       31 c0                   xor    %eax,%eax
    1002:       c3                      ret

```
<!-- .element: class="r-stretch" -->


