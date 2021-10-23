/*
 * World's worst linker script - VERY minimal to demo how to place config registers using
 * sections.
 *
 * Start with a real linker script for your platform...
 */
ENTRY(main)

MEMORY
{
  /* the config registers */
  config : ORIGIN = 0x0100, LENGTH = 0x8
  /* the rest of the flash*/
  prog (rx) : ORIGIN = 0x1000, LENGTH = 0x100000
}

SECTIONS
{
  /* place config registers in to the right memory location  */
  .config_registers : {
    KEEP(*(.config_registers))
  } > config

  /* everyting else */
  .text : { *(.text) *(.text.*) } > prog
  .data : { *(.data) } > prog
  .bss : { *(.bss) } > prog
  /DISCARD/ : { *(.*) }
}

