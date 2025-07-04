/*
  We cannot rely on the default linker script from a compiler since we have to be very
  strict with how memory is structured.
*/

ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
  /* First 1MB is reserved for BIOS, bootloader, and legacy hardware. */
  . = 0x0100000;

  .text :
  {
    *(.multiboot)
    *(.text*)     /* Code */
    *(.rodata)    /* Read only data */
  }

  .data  :
  {
    /*
      This sets up global object constructors (such as the GDT and IDT objects!)
      Normally, our compiler provides us with a linker script that does this.
      However, since we are writing our own linker script, we must create a table of
      constructors to be called before kernel_main().
    */
    start_ctors = .;
    KEEP(*( .init_array )); /* This array contains pointers to constructors. */

    /* The line below ensures that earlier constructors are called before later constructors. */
    KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    end_ctors = .;

    *(.data)
  }

  .bss  :
  {
    /* For uninitialized global/static variables */
    *(.bss)
  }


  /*  Since the kernel doesn't exist destructors aren't needed. */
  /DISCARD/ : { *(.fini_array*) *(.comment) }
}
