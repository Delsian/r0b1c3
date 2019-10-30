/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x27000, LENGTH = 0xd9000
  RAM (rwx) :  ORIGIN = 0x20002300, LENGTH = 0x3dd00
}

INCLUDE "nrf52.ld"
