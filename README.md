# DonkeyOS (a work in progress)

A minimal hobby OS project in C++ using GRUB, QEMU, and Vagrant.

## Prerequisites

- **Host (Windows)**
  - VirtualBox 7.x installed
  - Vagrant 2.x installed
- **Guest (Ubuntu 22.04 via Vagrant)**
  - build-essential, gcc-multilib, g++-multilib
  - nasm (via `as --32`), bison, flex, texinfo
  - grub-pc-bin, xorriso
  - qemu-system-i386, gdb-multiarch

All guest dependencies are automatically provisioned by the Vagrantfile.

## To Start

1. **Bring up the VM**  
   ```
   vagrant up
   ```
2. **SSH into the VM**  
   ```
   vagrant ssh
   ```
3. **Build and run the OS!**  
   ```
   cd /osdev
   make run
   ```

#### Note:
If you want to use the mouse, you may have to run the OS through virtual box directly.

## Features
- Print "Hello World!" (Complete)
- GDT (Complete)
- IDT (Complete)
- Drivers (WIP)
   - Keyboard Input (Complete)
   - Mouse Input (WIP)
   - Driver Management (WIP)
- PCI (WIP)
- Paging (Not started)
- Graphics (Not started)
- Task Scheduling (Not started)
- Memory Management (Not started)
- Networking (Not started)
- Custom Bootloader (Not started)
