# DonkeyOS

A minimal hobby OS project in C++

## Features
- 🟢 Print "Hello World!" (Complete)
- 🟢 GDT (Complete)
- 🟢 IDT (Complete)
- 🟡 Drivers (WIP)
   - 🟢 Keyboard Input (Complete)
   - 🟡 Mouse Input (WIP)
   - 🟢 Driver Management (Complete)
- 🟢 PCI (Complete)
- 🟢 Task Scheduling (Complete)
- 🟡 Memory Management (WIP)
- 🟡 Networking (WIP)
- 🔴 Paging (Not started)
- 🔴 Graphics (Not started)
- 🔴 Custom Bootloader (Not started)

## To Start

#### Option 1: Use Virtual Box

1. **Ensure you have virtual box installed**  

   Get it from here: https://www.virtualbox.org/

2. **Run the setup.sh script**  

   If you're in a Windows environment, you can run the script in WSL.

-----------------------------------------------------------------------------------------------------------------------

#### Option 2: Use Qemu (note that some features like the mouse and network may not work):
1. **Ensure you have all the necessary dependencies**

   If you're on Windows, you can provision a VM with all the dependencies using Vagrant after installing it by running:

   ```
   vagrant up
   ```

   and then to connect:

   ```
   vagrant ssh
   ```

3. **Build and run the OS!**  
   ```
   cd /osdev
   make run-qemu
   ```

# Documentation / Notes

## Booting and loading the kernel

A computer has a motherboard. Embedded on the motherboard is a piece of firmware called the BIOS (Basic Input Output System) or UEFI (Unified Extensible Firmware Interface) on newer systems. It is the first software that is run when a computer is powered on.
When the computer is started, the CPU begins executing firmware code directly from ROM/flash memory (separate from RAM) where the BIOS is stored. The CPU starts at a predetermined memory address (known as the reset vector) where the BIOS code begins execution.

The BIOS firmware performs POST (Power-On Self-Test) to check hardware, then locates a bootable device (hard drive, USB, CD, etc.) and loads the boot sector/Master Boot Record (MBR), which is the first 512 bytes of the storage device into memory at address 0x7C00. This loaded section contains bootloader code (or a simple boot program). The firmware then tells the CPU to jump to this bootloader code at 0x7C00.

The bootloader may print a list of operating systems on the screen (like GRUB does), but simpler bootloaders just boot a single OS automatically. It copies the kernel into memory at the appropriate location and then tells the CPU to jump to the kernel's entry point.

Note: The bootloader sets up a stack pointer because it needs a stack to function properly. However, the kernel will later establish its own stack and memory management once it takes control.

Because we don't want to spend too much time and effort building a custom bootloader for our yet-to-exist OS, we will use GRUB for now. In order for the bootloader to know that our kernel is a kernel (and not just random data in memory), we provide a magic number for it to look for. Additionally, we pass it flags that specify what information we would like to receive from it (more on this later). Finally, we also add a checksum so that the bootloader can validate that the multiboot header is properly formed (and not just a coincidence from random data). This logic is present at the very beginning of our `loader.s` file.
