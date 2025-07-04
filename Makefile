# Just for future reference, in case you forget:
# $@ evaluates to the target.
# $< evaluates to the first prerequisite  .
# $^ evaluates to all prerequisites.
# -c is for source files and -o is for the output file.

# =============================================================================
# Tool Configuration
# =============================================================================

AS      := as       # GNU assembler
ASFLAGS := --32     # Generate 32-bit code

CC      := g++      # C++ compiler  
LD      := ld       # GNU linker

# =============================================================================
# Directory Structure
# =============================================================================

BUILD_DIR   := build
SRC_DIR     := src
INCLUDE_DIR := include

# =============================================================================
# Compiler Flags
# =============================================================================

# Compiler flags explained:
# -m32: Tells the compiler to generate 32-bit code.
# -ffreestanding: Freestanding environment (no standard library available).
# -fno-use-cxa-atexit: Disable __cxa_atexit for destructors since the kernel doesn't have it.
# -nostdlib: We don't want to link against standard library.
# -fno-builtin: We don't have access to these built in functions.
# -fno-exceptions: Disable C++ exceptions (our kernel can't handle them).
# -fno-rtti: Disable RunTime Type Information (depends on stdlib components).
# -fno-leading-underscore: Tells compiler to not add leading underscores to symbol names.
# -Wno-write-strings: Disable warnings about writing to string literals (for now).
# -I$(INCLUDE_DIR): Adds the include directory to header search path.

CFLAGS := -m32 \
          -ffreestanding \
          -fno-use-cxa-atexit \
          -nostdlib \
          -fno-builtin \
          -fno-exceptions \
          -fno-rtti \
          -fno-leading-underscore \
          -Wno-write-strings \
          -I$(INCLUDE_DIR)

# =============================================================================
# Linker Flags  
# =============================================================================

# Linker flags:
# -m elf_i386: Tells the linker to generate 32-bit ELF output.
# -T linker.ld: Use the specified linker script we wrote.
LDFLAGS := -m elf_i386 -T linker.ld

# =============================================================================
# Object Files
# =============================================================================

# Assembly object files
ASM_OBJECTS := $(BUILD_DIR)/loader.o \
               $(BUILD_DIR)/interruptstubs.o

# C++ object files  
CPP_OBJECTS := $(BUILD_DIR)/gdt.o \
               $(BUILD_DIR)/port.o \
			   $(BUILD_DIR)/terminal.o \
               $(BUILD_DIR)/interrupts.o \
               $(BUILD_DIR)/keyboard.o \
               $(BUILD_DIR)/kernel.o

# All object files
ALL_OBJECTS := $(ASM_OBJECTS) $(CPP_OBJECTS)

# =============================================================================
# Phony Targets
# =============================================================================

# Tell Make that these aren't real files, just commands hehe.
.PHONY: all iso run clean

# =============================================================================
# Build Targets
# =============================================================================

all: $(BUILD_DIR)/kernel.bin

# Assembly source files
$(BUILD_DIR)/loader.o: $(SRC_DIR)/loader.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/interruptstubs.o: $(SRC_DIR)/interruptstubs.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

# C++ source files
$(BUILD_DIR)/gdt.o: $(SRC_DIR)/gdt.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/port.o: $(SRC_DIR)/port.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/terminal.o: $(SRC_DIR)/terminal.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/interrupts.o: $(SRC_DIR)/interrupts.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/keyboard.o: $(SRC_DIR)/keyboard.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# This links all the object files together to produce the kernel binary code.
$(BUILD_DIR)/kernel.bin: $(ALL_OBJECTS) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) $^ -o $@

# This creates a bootable ISO image.
$(BUILD_DIR)/os.iso: $(BUILD_DIR)/kernel.bin grub.cfg | $(BUILD_DIR)
	# Create ISO directory structure
	# The -p flag creates parent directories if they don't exist
	mkdir -p iso_dir/boot/grub
	cp $(BUILD_DIR)/kernel.bin iso_dir/boot/kernel.bin
	cp grub.cfg iso_dir/boot/grub/
	
	# Generate bootable ISO using GRUB
	grub-mkrescue -o $(BUILD_DIR)/os.iso iso_dir

# =============================================================================
# Convenience Targets
# =============================================================================

# Build the ISO image.
iso: $(BUILD_DIR)/os.iso

# Run the OS using the QEMU emulator.
# QEMU treats os.iso as if it's a physical CD-ROM inserted into a virtual computer.
run: iso
	qemu-system-i386 -cdrom $(BUILD_DIR)/os.iso -curses

# Clean all build artifacts.
clean:
	rm -rf $(BUILD_DIR) iso_dir

# Create the build directory.
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
